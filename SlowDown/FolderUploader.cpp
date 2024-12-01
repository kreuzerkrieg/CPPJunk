#include "FolderUploader.h"
#include "S3MultipartUploader.h"
#include "Utils.h"
#include <filesystem>
#include <fstream>

FolderUploader::FolderUploader(const std::string& local_path_, const std::string& s3_fqp_)
    : local_path(local_path_), s3_fqp(s3_fqp_), max_buffers(1_GiB), part_size(20_MiB)
{
}

void FolderUploader::Upload()
{
    struct Uploader
    {
        size_t upload_size = 0;
        std::unique_ptr<S3MultipartUploader> uploader = nullptr;
    };
    namespace fs = std::filesystem;

    std::string bucket_name;
    std::string object_name;
    s3fqn2parts(s3_fqp, bucket_name, object_name);

    std::deque<Uploader> uploaders;
    for(auto& entry : fs::recursive_directory_iterator(local_path))
    {
        if(fs::is_regular_file(entry))
        {
            std::string stem_to_object = entry.path().string().substr(local_path.length() + 1);
            while(buffers_inflight > max_buffers && !uploaders.empty())
            {
                buffers_inflight -= uploaders.front().upload_size;
                uploaders.front().uploader->finalizeUpload();
                uploaders.pop_front();
            }

            std::ifstream file(entry.path(), std::ios::binary);
            std::vector<char> buffer;
            uploaders.emplace_back();
            uploaders.back().uploader = std::make_unique<S3MultipartUploader>(bucket_name, fs::path(object_name) / stem_to_object);
            while(!file.eof())
            {
                buffer.clear();
                buffer.resize(part_size);
                file.read(buffer.data(), buffer.size());
                buffer.resize(file.gcount());
                buffer.shrink_to_fit();
                uploaders.back().upload_size += buffer.size();
                buffers_inflight += buffer.size();
                uploaders.back().uploader->uploadPart(std::move(buffer));
                while(buffers_inflight > max_buffers && uploaders.size() > 1)
                {
                    if(uploaders.size() > 1)
                    {
                        while(buffers_inflight > max_buffers && uploaders.size() > 1)
                        {
                            buffers_inflight -= uploaders.front().upload_size;
                            uploaders.front().uploader->finalizeUpload();
                            uploaders.pop_front();
                        }
                    }
                    else if(uploaders.size() == 1)
                    {
                        while(buffers_inflight > max_buffers)
                        {
                            auto freed_bytes = uploaders.front().uploader->finalizePart();
                            if(freed_bytes == 0)
                            {
                                throw std::logic_error(
                                    "All buffer exhausted despite all S3 upload parts were finalized. Smells like logical bug.");
                            }
                            buffers_inflight -= freed_bytes;
                        }
                    }
                }
            }
        }
    }
    while (!uploaders.empty())
    {
        uploaders.front().uploader->finalizeUpload();
        uploaders.pop_front();
    }
}

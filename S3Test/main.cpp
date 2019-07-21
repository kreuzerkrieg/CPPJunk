#include "DirectoryIterator.h"
#include "Downloader.h"
#include "S3File.h"
#include "S3LazyListRetriever.h"
#include "Uploader.h"
#include <aws/core/Aws.h>

int main(int argc, char** argv)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    /*    fs::path path("s3:/dev-shadow/./data/default//S3/ontime_replicated/tmp_fetch_198711_0_0_0/.");
        fs::path clean;
        for(const auto& part : path)
        {
            if(part == ".")
            {
                continue;
            }
            if(part == "..")
            {
                clean = clean.parent_path();
                continue;
            }
            clean /= part;
        }
        clean /= "";
        std::cout << path.string() << std::endl;
        std::cout << clean.string() << std::endl;
        std::cout << path.generic_string() << std::endl;
        std::cout << path.parent_path().string() << std::endl;
        std::cout << path.stem().string() << std::endl;
        std::cout << path.native() << std::endl;
        std::cout << path.root_path().native() << std::endl;
        std::cout << path.root_directory().native() << std::endl;
        std::cout << path.root_name().native() << std::endl;
        std::cout << fs::absolute(path).string() << std::endl;
        //    std::cout << fs::canonical(path).string() << std::endl;
        std::cout << fs::system_complete(path).string() << std::endl;

        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei/"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }
        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }

        {
            S3File f1(fs::path("s3://dev-shadow/ein/zwei.bin"));
            std::cout << f1.getFQName() << " is a " << (f1.isFile() ? " file." : " folder.") << std::endl;
        }

        {
            // Test folder copy to another folder
            auto source = S3File(std::string("s3://dev-shadow/one/"));
            source.copy(std::string("s3://dev-shadow/ein/zwei/"));
        }*/

    // fs::path test_path("s3://dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198711_0_0_0/columns.txt");

    //    S3File source_file(std::string("s3:/dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198711_0_0_0/columns.txt"));
    //    std::cout << source_file.exists() << std::endl;
    //    source_file.print();
    //    // Test copy
    //    auto copied = source_file.copy("s3://dev-shadow/ein/zwei/test_file.txt");
    //    // Test copy to another folder
    //    auto ccopied = copied.copy("s3://dev-shadow/one/two/test_file.txt");
    //
    //    // Test folder copy to another folder
    //    auto source = S3File(std::string("s3://dev-shadow/one/"));
    //    source.copy(std::string("s3://dev-shadow/ein/zwei/"));
    //
    //    // Test file rename
    //    ccopied.rename(std::string("s3://dev-shadow/one/two/renamed_file.txt"));
    //
    //    // Test folder rename
    //    S3File source_folder(fs::path("s3://dev-shadow/ein/zwei/"));
    //    auto new_folder = source_folder.rename(std::string("s3://dev-shadow/ein/dos/"));
    //
    //    // Cleanup and test deletion
    //    new_folder.remove();

    //    fs::path p("s3://dev-shadow/data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0/AirTime.bin");
    //    S3File file1(p);
    //    file1.print();
    //    fs::path new_name("s3://dev-mini-test/Renamed.file");
    //
    //    S3File file3 = file1.rename(new_name);
    //    file1.print();
    //    file3.print();
    //    file3.remove();
    //    file3.print();
    //    S3File file4(new_name);
    //    file4.print();

    //    S3LazyListRetriever retriever("dev-shadow", "data/default/S3/ontime_replicated/tmp_fetch_198802_0_0_0");
    //    auto files = retriever.next(15);
    //    auto results = files.get();
    //
    //    fs::path p1("s3://dev-shadow/data/default/S3");
    //    S3File file2(p1);
    //    file2.print();
    //    file2.remove();
    //    file2.print();
    //
    //    fs::path n;
    //
    //    auto it = p.begin();
    //    ++it;
    //    for(; it != p.end(); ++it)
    //    {
    //        n /= *it;
    //    }
    //    auto a = fs::absolute(p);
    //    auto b = p.stem();
    //
    //    std::string bucket_name = "dev-shadow";
    //    std::string object_name = "./ein/zwei/drei/ontime0002.csv";
    //    std::string target_file = "/home/ernest/Downloads/tmp-ontime/x0002";
    //    UploadFile(bucket_name, object_name, target_file);
    //    DownloadFile(bucket_name, object_name, target_file + ".dwld");

    //    FileDownloader downloader;
    //    downloader.Download("s3://dev-full-ontime/x0002.7z", "/home/ernest/Downloads/x0002.7z");

    auto it = DirectoryIterator("s3://dev-shadow/data/default/S3/ontime_replicated/");
    for(; it != DirectoryIterator(); ++it)
    {
        std::cout << *it << std::endl;
    }

    for(auto& item : DirectoryIterator("s3://dev-shadow/data/default/S3/ontime_replicated/"))
    {
        std::cout << (item.filename() != "." ? item.filename() : item.parent_path().filename()) << std::endl;
    }
    Aws::ShutdownAPI(options);
}

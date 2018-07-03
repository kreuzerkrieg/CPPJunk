#include "NVMeStriper.h"
#include "NVMeManager.h"

//#define BOOST_THREAD_PROVIDES_FUTURE
//#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
//#define BOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY
//
//#include <boost/thread/future.hpp>

NVMeStriper::NVMeStriper(const std::string& deviceName, NVMeManager& nvmeManager) : nvmeManager(nvmeManager),
                                                                                    deviceName(deviceName) {
}

std::future<void> NVMeStriper::Read(Baddr offset, byte* buffer, uint32_t nbytes) {
    return nvmeManager.Read(deviceName, offset, buffer, nbytes);
}

std::future<void> NVMeStriper::Write(Baddr offset, const byte* buffer, uint32_t nbytes) {
    return nvmeManager.Write(deviceName, offset, const_cast<byte*>(buffer), nbytes);
}

std::future<void> NVMeStriper::ReadV(Baddr offset, iovec* iovec, int niovec) {
    return std::future<void>();
//    std::vector<std::future<void>> futures;
//    for (int i = 0; i < niovec; ++i) {
//        futures.emplace_back(nvmeManager.Read(deviceName, offset, iovec[i].iov_base, iovec[i].iov_len));
//        offset += iovec[i].iov_len;
//    }
//    std::promise<void> promise;
//    auto future = promise.get_future();
//    boost::when_all(futures.begin(), futures.end()).then(
//            [promise{std::move(promise)}](auto&&) mutable { promise.set_value(); }).set_async();
//    return future;
}

std::future<void> NVMeStriper::WriteV(Baddr offset, iovec* iovec, int niovec) {
    return std::future<void>();
}

uint64_t NVMeStriper::GetSize() const {
    return nvmeManager.GetSize(deviceName);
}

unsigned int NVMeStriper::GetBlockSize() const {
    return nvmeManager.GetBlockSize(deviceName);
}

uint64_t NVMeStriper::GetOptIoSize() const {
    return 1;
}

bool NVMeStriper::IsTrimmed(Baddr offset) const {
    return false;
}

void NVMeStriper::Trim(Baddr addr, uint64_t baddrsCount) {

}


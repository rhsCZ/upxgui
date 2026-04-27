#include "upx_inproc.h"

#include <cstring>
#include <memory>
#include <vector>

int upx_main(int argc, char *argv[]);

namespace upxgui {

int RunEmbeddedUpx(const std::vector<std::string> &arguments) noexcept {
    std::vector<std::unique_ptr<char[]>> storage;
    std::vector<char *> argv;
    storage.reserve(arguments.size() + 1);
    argv.reserve(arguments.size() + 1);

    for (const std::string &argument : arguments) {
        std::unique_ptr<char[]> buffer(new char[argument.size() + 1]);
        memcpy(buffer.get(), argument.c_str(), argument.size() + 1);
        argv.push_back(buffer.get());
        storage.push_back(std::move(buffer));
    }

    if (argv.empty()) {
        static char default_name[] = "upxgui";
        argv.push_back(default_name);
    }

    try {
        return upx_main(static_cast<int>(argv.size()), argv.data());
    } catch (int exit_code) {
        return exit_code;
    } catch (...) {
        return 1;
    }
}

} // namespace upxgui

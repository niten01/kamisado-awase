#include <drogon/drogon.h>
#include <filesystem>

auto main(int /*argc*/, char** argv) -> int {
  drogon::app().addListener("0.0.0.0", 8081);
  std::filesystem::path binPath{ argv[0] };
  drogon::app().loadConfigFile(binPath.parent_path() / "config.yaml");
  drogon::app().run();
  return 0;
}

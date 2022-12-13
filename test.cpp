

#include "mkn/kul/signal.hpp"
#include "mkn/kul/yaml.hpp"
#include <maiken.hpp>

std::string const yArgs = "with: numpy";

int main(int argc, char* argv[]) {
  mkn::kul::Signal sig;
  try {
    YAML::Node node = mkn::kul::yaml::String(yArgs).root();
    char* argv2[2] = {argv[0], (char*)"-O"};
    auto app = (maiken::Application::CREATE(2, argv2))[0];
    auto loader(maiken::ModuleLoader::LOAD(*app));
    loader->module()->init(*app, node);
    loader->module()->compile(*app, node);
    loader->module()->link(*app, node);
    loader->module()->pack(*app, node);
    loader->unload();
    for(auto const inc : app->includes()) KLOG(INF) << inc.first;
  } catch (mkn::kul::Exception const& e) {
    KLOG(ERR) << e.what();
    return 2;
  } catch (std::exception const& e) {
    KERR << e.what();
    return 3;
  } catch (...) {
    KERR << "UNKNOWN EXCEPTION TYPE CAUGHT";
    return 5;
  }
  return 0;
}

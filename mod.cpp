/**
Copyright (c) 2013, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <cstdint>
#include "maiken/module/init.hpp"

#include <unordered_set>

namespace mkn {
namespace lang {

class SwigModule : public maiken::Module {
 private:
  bool pyconfig_found = 0;
   std::string HOME, SWIG = "swig",
               PATH = mkn::kul::env::GET("PATH");

 protected:

  static void VALIDATE_NODE(const YAML::Node& node) {
    using namespace mkn::kul::yaml;
    Validator(
      {
        NodeValidator("src", 1), // TODO : add regex
        NodeValidator("inc"),   // TODO : add regex
        NodeValidator("conf"),
        NodeValidator("outdir"),
        NodeValidator("objdir"),
        NodeValidator("objfile"),
      }
    ).validate(node);
  }

 public:
  SwigModule(){
    if(mkn::kul::env::EXISTS("SWIG")) SWIG = mkn::kul::env::GET("SWIG");
  }
  void compile(maiken::Application& a, const YAML::Node& node)
      KTHROW(std::exception) override {
    VALIDATE_NODE(node);

    std::string conf = "-python -c++";
    if(node["conf"]) conf = node["conf"].Scalar();
    std::stringstream incs;
    if(node["inc"])
      for(const auto inc : mkn::kul::cli::asArgs(node["inc"].Scalar()))
        incs << "-I" << inc << " ";

    for(const auto source : mkn::kul::cli::asArgs(node["src"].Scalar())){
      mkn::kul::File src(source);
      if(!src) {
        KERR << "WARNING: Source does not exist: " << src.full();
        continue;
      }

      std::string outdir = src.dir().escm(), objdir = src.dir().escm();
      if(node["outdir"]) outdir = node["outdir"].Scalar();
      if(node["objdir"]) outdir = node["objdir"].Scalar();

      mkn::kul::File objfile(src.name()+".cpp", objdir);
      if(node["objfile"]) objfile = mkn::kul::File(node["objfile"].Scalar(), objdir);

      mkn::kul::Process p(SWIG);
      p << conf << incs.str();
      p << "-outdir" << outdir;
      p << "-o" << objfile.escm() << src.escm();
      KLOG(DBG) << p;
      p.start();
    }

    a.m_cInfo.lib_ext = mkn::kul::String::LINES(extension())[0];  // drop EOL
    a.m_cInfo.lib_prefix = "";
      KLOG(TRC);
    a.mode(maiken::compiler::Mode::SHAR);
      KLOG(TRC);
  }
protected:
  std::string extension(){
      auto pyconfig = mkn::kul::env::GET("PY3_CONFIG", "python3-config");

#if defined(_WIN32)
      auto pyconfig_found = false;  // doesn't exist on windows (generally)
#else
      auto pyconfig_found = mkn::kul::env::WHICH(pyconfig);
#endif

      std::string extension;
      if (pyconfig_found) {
        // mkn::kul::os::PushDir pushd(a.project().dir());
        mkn::kul::Process p(pyconfig);
        mkn::kul::ProcessCapture pc(p);
        p << "--extension-suffix";
        p.start();
        extension = pc.outs();
      } else {
        auto py = mkn::kul::env::GET("PYTHON", "python3");
        mkn::kul::Process p(py);
        mkn::kul::ProcessCapture pc(p);
        p << "-c"
          << "\"import sysconfig; "
             "print(sysconfig.get_config_var('EXT_SUFFIX'))\"";
        p.start();
        extension = pc.outs();
      }
      return extension;
  }
};
}  // namespace lang
}  // namespace mkn

extern "C" KUL_PUBLISH maiken::Module* maiken_module_construct() {
  return new mkn::lang::SwigModule;
}

extern "C" KUL_PUBLISH void maiken_module_destruct(maiken::Module* p) {
  delete p;
}

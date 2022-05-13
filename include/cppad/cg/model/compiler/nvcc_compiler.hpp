#ifndef CPPAD_CG_NVCC_COMPILER_INCLUDED
#define CPPAD_CG_NVCC_COMPILER_INCLUDED

namespace CppAD {
namespace cg {

/**
 * NVCCC compiler class used to create a dynamic library
 *
 */
template<class Base>
class NvccCompiler : public AbstractCCompiler<Base> {
public:
  
  NvccCompiler(const std::string& nvccPath = "/usr/bin/nvcc") :
    AbstractCCompiler<Base>(nvccPath) {
    
    //this->_compileFlags.push_back("-XCompiler"); // send flags directly to compiler.
    //TODO: add optimization params for device code.
    // https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html#command-option-description
  }
  
  NvccCompiler(const NvccCompiler& orig) = delete;
  NvccCompiler& operator=(const NvccCompiler& rhs) = delete;
  
  /**
   * Creates a dynamic library from a set of object files
   *
   * @param library the path to the dynamic library to be created
   */
  void buildDynamic(const std::string& library,
                    JobTimer* timer = nullptr) override {
    
    //Linker information
    //TODO: Setup Linker information
    
    std::vector<std::string> args;
    //TODO: Feed args
    
    if (timer != nullptr) {
      timer->startingJob("'" + library + "'", JobTimer::COMPILING_DYNAMIC_LIBRARY);
    }
    else if (this->_verbose) {
      std::cout << "building library '" << library << "'" << std::endl;
    }
    
    // Call the executable with args.
    system::callExecutable(this->_path, args);
    
    if (timer != nullptr) {
      timer->finishedJob();
    }
    
  }
  
  virtual ~NvccCompiler() = default;

protected:

  /**
   * Compiles a single source file into an object file
   *
   * @param source the content of the source file
   * @param output the compiled output file name (the object file path)
   */
  void compileSource(const std::string& source,
                     const std::string& output,
                     bool posIndepCode) override {
    std::vector<std::string> args;
    args.push_back("--compiler-options");
    if (posIndepCode) {
      args.push_back("-fPIC"); // position-independent code for dynamic linking
    }    
    args.push_back("-x");
    args.push_back("cu");
    args.push_back("-rdc=true");
    args.insert(args.end(), this->_compileFlags.begin(), this->_compileFlags.end());
    args.push_back("-c");
    args.push_back("-");
    args.push_back("-o");
    args.push_back(output);
    
    system::callExecutable(this->_path, args, nullptr, &source);
  }
  

  void compileFile(const std::string& path,
                   const std::string& output,
                   bool posIndepCode) override {
    std::vector<std::string> args;
    args.push_back("--compiler-options");
    if (posIndepCode) {
      args.push_back("-fPIC"); // position-independent code for dynamic linking
    }
    args.push_back("-x");
    args.push_back("cu");
    args.push_back("-rdc=true");
    args.insert(args.end(), this->_compileFlags.begin(), this->_compileFlags.end());
    args.push_back("-c");
    args.push_back("-");
    args.push_back("-o");
    args.push_back(output);    
    system::callExecutable(this->_path, args);
  }
};

} // namespace cg
} //namespace CppAD
  
#endif // CPPAD_CG_NVCC_COMPILER_INCLUDED

env = Environment()
Export('env')

env.Prepend(
        CXXFLAGS = [
            "-std=c++11",
            "-Wall",
            "-Wextra",
            ],
        CPPFLAGS = [
            "-g", "-DDEBUG" ])
SConscript('src/SConscript', variant_dir='build/raft')
SConscript('test/SConscript', variant_dir='build/test')

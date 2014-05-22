env = Environment(
       tools=['default', 'protoc'])

Export('env')

env.Prepend(
        CXXFLAGS = [
            "-std=c++11",
            "-Wall",
            "-Wextra",
            ],
        CPPFLAGS = [
            "-g", "-DDEBUG" ])

raft_o = []
Export('raft_o')

SConscript('src/SConscript', variant_dir='build/raft')
SConscript('test/SConscript', variant_dir='build/test')
Clean('.', 'build')
PhonyTargets(tags = "ctags -R --c++-kinds=+p --fields=+iaS --extra=+q /usr/include/boost/ .")

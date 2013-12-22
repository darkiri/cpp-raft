#ifndef RAFT_NODE
#define RAFT_NODE

namespace raft {
    enum NodeState {
        Follower = 0,
        Candidate = 1,
        Leader = 2
    };

    class Node {
        public:
            Node() {}
            NodeState GetState(){
                return _state;
            }
        private:
            Node(Node&);
            NodeState _state;
    };
}
#endif

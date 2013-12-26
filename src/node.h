#ifndef RAFT_NODE
#define RAFT_NODE

namespace raft {
    enum NodeState {
        FOLLOWER = 0,
        CANDIDATE = 1,
        LEADER = 2
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

#include "path.hpp"


std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *> paths::Full::Connect(tensor::Shape &send, tensor::Shape &recv, bool same){
    tensor::Int32 *sendn, *recvn;
    tensor::Bits *cons;
    
    auto tensorTuple = paths::NewTensors(send, recv);
    sendn = std::get<0>(tensorTuple);
    recvn = std::get<1>(tensorTuple);
    cons = std::get<2>(tensorTuple);

    cons->SetAll(true);

    int nsend = send.Len();
    int nrecv = recv.Len();

    if (same && !SelfCon) { // turn off all self connections
        for (int i = 0; i < nsend; i++) {
            int offset = i*nsend + i;
            cons->SetValue(offset, false);
        }
    }

    recvn->SetAll(nsend);
    sendn->SetAll(nrecv);
    // TODO: Investigate the purpose of these tensors... why do the elements carry redundant information?

    return std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *>(sendn, recvn, cons);
}

// NewTensors returns the tensors used for Connect method, based on layer sizes
std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *> paths::NewTensors(tensor::Shape &send, tensor::Shape &recv) {
    tensor::Int32 *sendn = new tensor::Int32(send);
    tensor::Int32 *recvn = new tensor::Int32(send);
    tensor::Shape csh = tensor::AddShapes(recv, send);
    tensor::Bits *cons = new tensor::Bits(csh);
    return std::tuple<tensor::Int32 *, tensor::Int32 *, tensor::Bits *>(sendn, recvn, cons);
}

void paths::GaussTopo::Defaults(){
    Sigma = 0.6;
    Wrap = true;
    CtrMove = 1;
}

bool paths::PoolTile::HasTopoWeights() {
    return GaussFull.On || GaussInPool.On || SigFull.On || SigInPool.On;
}

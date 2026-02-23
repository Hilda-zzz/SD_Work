// ===== Buffers =====
RWStructuredBuffer<uint> Counters : register(u0);
//   [0] aliveCount              
//   [1] deadCount               
//   [2] emitCount               
//   [3] aliveCountAfterSim      

[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    Counters[0] = Counters[3];
    Counters[3] = 0;
}
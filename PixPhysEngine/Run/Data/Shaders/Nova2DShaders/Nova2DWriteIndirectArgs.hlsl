RWStructuredBuffer<uint> Counters : register(u0);       // [aliveCount, deadCount, emitCount, aliveCountAfterSim]
RWByteAddressBuffer IndirectArgs : register(u1);        // Indirect draw args

[numthreads(1, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    IndirectArgs.Store(0,  Counters[3] * 6u);  // IndexCountPerInstance
    IndirectArgs.Store(4,  1u);                 // InstanceCount = 1
    IndirectArgs.Store(8,  0u);                 // StartIndexLocation
    IndirectArgs.Store(12, 0u);                 // BaseVertexLocation
    IndirectArgs.Store(16, 0u);                 // StartInstanceLocation
}
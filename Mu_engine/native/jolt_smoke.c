/* jolt_smoke.c — headless physics smoke test (ENGINE.md §17 test-physics
 * task). Brings JoltC 5.6.0 up, drops a dynamic box onto a static floor,
 * steps 60 frames of gravity, checks the box actually fell and stopped
 * above the floor, prints the final height, tears everything down.
 *
 *   clang -std=c11 native/jolt_smoke.c \
 *         -I third_party/JoltPhysicsC/JoltC/include \
 *         -o build/jolt_smoke -L build/native/joltc -lJoltC -lJolt \
 *         -lstdc++ -lm -lpthread -ldl && timeout 15 build/jolt_smoke
 *
 * Exits non-zero on any misbehaviour so it is CI-ready once Jolt is built
 * on this machine (native builds are -j1 only — see coral.crlb). */

#include <JoltC/joltc.h>
#include <stdio.h>

enum { LAYER_NON_MOVING = 0, LAYER_MOVING = 1, NUM_LAYERS = 2 };

static uint32_t cbNumBroadPhaseLayers(void* ud)
{
    (void)ud;
    return 1;
}

static JoltC_BroadPhaseLayer cbMapLayer(void* ud, JoltC_ObjectLayer layer)
{
    (void)ud;
    (void)layer;
    return 0; /* one broadphase layer for the smoke */
}

static JoltC_Bool cbObjectVsBroadPhase(void* ud, JoltC_ObjectLayer ol, JoltC_BroadPhaseLayer bp)
{
    (void)ud;
    (void)ol;
    (void)bp;
    return JOLTC_TRUE; /* at smoke scale everything overlaps */
}

static JoltC_Bool cbObjectPairs(void* ud, JoltC_ObjectLayer a, JoltC_ObjectLayer b)
{
    (void)ud;
    if (a == LAYER_MOVING && b == LAYER_MOVING) return JOLTC_FALSE;
    return JOLTC_TRUE;
}

int main(void)
{
    JoltC_RegisterDefaultAllocator();
    if (!JoltC_Init())
    {
        printf("jolt_smoke: JoltC_Init failed: %s\n", JoltC_GetLastError());
        return 1;
    }
    JoltC_CreateFactory();
    JoltC_RegisterTypes();

    JoltC_TempAllocator* temp = JoltC_TempAllocator_Create(10u * 1024u * 1024u);
    JoltC_JobSystem* jobs = JoltC_JobSystemThreadPool_Create(2048, 16, 0);

    JoltC_BroadPhaseLayerInterface* bpIface =
        JoltC_BroadPhaseLayerInterface_Create(cbNumBroadPhaseLayers, cbMapLayer, 0);
    JoltC_ObjectVsBroadPhaseLayerFilter* ovbFilter =
        JoltC_ObjectVsBroadPhaseLayerFilter_Create(cbObjectVsBroadPhase, 0);
    JoltC_ObjectLayerPairFilter* pairFilter =
        JoltC_ObjectLayerPairFilter_Create(cbObjectPairs, 0);

    JoltC_PhysicsSystem* phys = JoltC_PhysicsSystem_Create();
    JoltC_PhysicsSystem_Init(phys, 1024, 0, 1024, 1024, bpIface, ovbFilter, pairFilter);
    JoltC_BodyInterface* bodies = JoltC_PhysicsSystem_GetBodyInterface(phys);

    JoltC_Quat identity = { 0.0f, 0.0f, 0.0f, 1.0f };

    /* static floor: 100x2x100 slab centred on the origin */
    JoltC_Vec3 floorHalf = { 50.0f, 1.0f, 50.0f };
    const JoltC_Shape* floorShape = JoltC_BoxShape_Create(floorHalf, 0.05f);
    JoltC_RVec3 floorPos = { 0.0f, -1.0f, 0.0f };
    JoltC_BodyCreationSettings* floorCfg =
        JoltC_BodyCreationSettings_Create3(floorShape, floorPos, identity,
                                           JOLTC_MOTION_TYPE_STATIC, LAYER_NON_MOVING);
    JoltC_BodyID floorId =
        JoltC_BodyInterface_CreateAndAddBody(bodies, floorCfg, JOLTC_ACTIVATION_DONT_ACTIVATE);
    JoltC_BodyCreationSettings_Destroy(floorCfg);

    /* dynamic 1m box dropped from y = 10 */
    JoltC_Vec3 boxHalf = { 0.5f, 0.5f, 0.5f };
    const JoltC_Shape* boxShape = JoltC_BoxShape_Create(boxHalf, 0.05f);
    JoltC_RVec3 boxPos = { 0.0f, 10.0f, 0.0f };
    JoltC_BodyCreationSettings* boxCfg =
        JoltC_BodyCreationSettings_Create3(boxShape, boxPos, identity,
                                           JOLTC_MOTION_TYPE_DYNAMIC, LAYER_MOVING);
    JoltC_BodyID boxId =
        JoltC_BodyInterface_CreateAndAddBody(bodies, boxCfg, JOLTC_ACTIVATION_ACTIVATE);
    JoltC_BodyCreationSettings_Destroy(boxCfg);

    JoltC_PhysicsSystem_OptimizeBroadPhase(phys);

    for (int i = 0; i < 60; i++)
        JoltC_PhysicsSystem_Update(phys, 1.0f / 60.0f, 1, temp, jobs);

    JoltC_RVec3 p = JoltC_BodyInterface_GetPosition(bodies, boxId);
    printf("jolt_smoke: box settled at y=%.3f (dropped from 10.0)\n", p.y);

    int failed = 0;
    if (p.y >= 9.9f)
    {
        printf("jolt_smoke: FAIL — box did not fall\n");
        failed = 1;
    }
    if (p.y < 0.4f)
    {
        printf("jolt_smoke: FAIL — box sank through the floor\n");
        failed = 1;
    }

    JoltC_BodyInterface_RemoveAndDestroyBody(bodies, boxId);
    JoltC_BodyInterface_RemoveAndDestroyBody(bodies, floorId);
    JoltC_PhysicsSystem_Destroy(phys);
    JoltC_ObjectLayerPairFilter_Destroy(pairFilter);
    JoltC_ObjectVsBroadPhaseLayerFilter_Destroy(ovbFilter);
    JoltC_BroadPhaseLayerInterface_Destroy(bpIface);
    JoltC_JobSystem_Destroy(jobs);
    JoltC_TempAllocator_Destroy(temp);
    JoltC_Shutdown();

    if (!failed) printf("jolt_smoke: PASS\n");
    return failed;
}

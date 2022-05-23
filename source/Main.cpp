#include "plugin.h"
#include "CPad.h"
#include "CHud.h"
#include "CText.h"
#include "CRenderer.h"

#ifdef GTASA
#include "CCheat.h"
#endif

const char* cheatString = "SDAEHELBBOB"; // BOBBLEHEADS
bool bobbleHeads = false;

using namespace plugin;

class BobbleHead {
public:
    static RpAtomic* IsSkinnedCb(RpAtomic* atomic, void* data) {
        RpAtomic** ret = (RpAtomic**)data;
        if (*ret)
            return NULL;
        assert(atomic->geometry->object.type = rpGEOMETRY);
        if (RpSkinGeometryGetSkin(atomic->geometry))
            *ret = atomic;
        return atomic;
    }

    static RpAtomic* IsClumpSkinned(RpClump* c) {
        RpAtomic* ret = NULL;
        assert(c->object.type == rpCLUMP);
        RpClumpForAllAtomics(c, IsSkinnedCb, &ret);
        return ret;
    }

    static RpAtomic* GetAnimHierarchyCallback(RpAtomic* atomic, void* data) {
        *(RpHAnimHierarchy**)data = RpSkinAtomicGetHAnimHierarchy(atomic);
        return NULL;
    }

    static RpHAnimHierarchy* GetAnimHierarchyFromSkinClump(RpClump* clump) {
        RpHAnimHierarchy* hier = NULL;
        RpClumpForAllAtomics(clump, GetAnimHierarchyCallback, &hier);
        return hier;
    }

    static int HAnimIDGetIndex(RpHAnimHierarchy* hier, int id) {
        for (int i = 0; i < hier->numNodes; i++)
            if (hier->pNodeInfo[i].nodeID == id)
                return i;
        return -1;
    }

    static void ResetCheat() {
        bobbleHeads = false;
    }

    static void ProcessBobbleHead(CPed* ped) {
        if (!bobbleHeads)
            return;

        const float scale = 3.0f;

        if (IsClumpSkinned(ped->m_pRwClump)) {
            RpHAnimHierarchy* hier = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
#ifdef GTA3
            const int boneTag = 9;
#elif GTAVC
            const int boneTag = 5;
#elif GTASA
            const int boneTag = 5;

            for (int i = boneTag + 1; i < 9; i++) {
                int index = HAnimIDGetIndex(hier, i);
                RwMatrix* mat = &hier->pMatrixArray[index];
                if (mat) {
                    RwV3d s = { scale, scale, scale };
                    RwMatrixScale(mat, &s, rwCOMBINEPRECONCAT);

                    // Fix forehead and jaw
                    if (i == 7 || i == 6 || i == 8) {
                        RwV3d t = { 0.0f, -(scale / 6.0f) / 10.0f, 0.0f};

                        if (i == 8) {
                            t.x = ((scale / 8.0f) / 10.0f) / 8.0f;
                            t.y /= 8.0f;
                        }
                        RwMatrixTranslate(mat, &t, rwCOMBINEPRECONCAT);
                    }
                }
            }
#endif
            int index = HAnimIDGetIndex(hier, boneTag);
            RwMatrix* mat = &hier->pMatrixArray[index];
            if (mat) {
                RwV3d s = { scale, scale, scale };
                RwMatrixScale(mat, &s, rwCOMBINEPRECONCAT);
            }
        }
        else {
#ifndef GTASA
            int node = 2;
            RwFrame* frame = ped->m_apFrames[node]->m_pFrame;
            if (frame) {
                RwMatrix* headMatrix = RwFrameGetMatrix(frame);
                if (headMatrix) {
                    CMatrix mat;
                    mat.m_pAttachMatrix = NULL;
                    mat.Attach(headMatrix, false);
                    mat.SetScale(scale);
                    mat.SetTranslateOnly(scale / 8.0f, 0.0f, 0.0f);
                    mat.UpdateRW();
                }
            }
#endif
        }
    }

    BobbleHead() {
#ifdef GTA3
        ThiscallEvent <AddressList<0x5841C7, H_CALL>, PRIORITY_AFTER, ArgPick2N<CPad*, 0, int, 1>, void(CPad*, int)> addToPCCheatString;
#elif GTAVC
        ThiscallEvent <AddressList<0x602BE7, H_CALL>, PRIORITY_AFTER, ArgPick2N<CPad*, 0, int, 1>, void(CPad*, int)> addToPCCheatString;
#endif
#ifdef GTASA
        plugin::Events::processScriptsEvent += [] {
#else
        addToPCCheatString += [](CPad* pad, int) {
#endif
#ifdef GTASA
            char* textOn = "CHEAT1";
            char* textOff = "CHEAT8";
            if (!strncmp(cheatString, CCheat::m_CheatString, 11)) {
                CCheat::m_CheatString[0] = NULL;
#else
            char* textOn = "CHEATON";
            char* textOff = "CHEATOF";
            if (!strncmp(cheatString, pad->KeyBoardCheatString, 11)) {
#endif
                bobbleHeads = bobbleHeads == false;
                CHud::SetHelpMessage(TheText.Get(bobbleHeads ? textOn : textOff), true
#ifdef GTAVC
                    , false
#endif
#ifdef GTASA
                    , false, false
#endif
                );
            }
        };

#ifdef GTA3
        CdeclEvent <AddressList<0x4A78A5, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> onPreRender;
#elif GTAVC
        CdeclEvent <AddressList<0x4CA255, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> onPreRender;
#elif GTASA
        CdeclEvent <AddressList<0x553A04, H_JUMP>, PRIORITY_AFTER, ArgPickNone, void()> onPreRender;
#endif
        onPreRender += [] {
            for (int i = 0; i < CRenderer::ms_nNoOfVisibleEntities; i++) {
                CEntity* e = CRenderer::ms_aVisibleEntityPtrs[i];
                if (e && (e->m_nType == ENTITY_TYPE_PED)) {
                    CPed* ped = static_cast<CPed*>(e);
                    ProcessBobbleHead(ped);
                }
            }
        };

#ifdef GTA3
        CdeclEvent <AddressList<0x48C7BE, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> reInitGameEvent;
#elif GTAVC
        CdeclEvent <AddressList<0x4A46F1, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> reInitGameEvent;
#else
        plugin::Events::reInitGameEvent += [] {
#endif
#ifndef GTASA
            reInitGameEvent += [] {
#endif
                ResetCheat();
            };
    }
} bobbleHead;

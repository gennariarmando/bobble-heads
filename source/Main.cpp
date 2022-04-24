#include "plugin.h"
#include "CPad.h"
#include "CHud.h"
#include "CText.h"

const char* cheatString = "SDAEHELBBOB"; // BOBBLEHEADS
bool bobbleHeads = false;

using namespace plugin;

class BobbleHead {
public:
	static void ProcessBobbleHead(CPed* ped) {
		if (!bobbleHeads)
			return;

		int node = 2;
		RwFrame* frame = ped->m_apFrames[node]->m_pFrame;

		if (frame) {
			RwMatrix* headMatrix = RwFrameGetMatrix(frame);

			if (headMatrix) {
				CMatrix mat;
				mat.m_pAttachMatrix = NULL;
				mat.Attach(headMatrix, false);
				mat.SetScale(3.0f);
				mat.SetTranslateOnly(0.4f, 0.0f, 0.0f);
				mat.UpdateRW();
			}
		}
	}

	BobbleHead() {
#ifdef GTA3
		ThiscallEvent <AddressList<0x5841C7, H_CALL>, PRIORITY_AFTER, ArgPick2N<CPad*, 0, int, 1>, void(CPad*, int)> addToPCCheatString;
#else
		ThiscallEvent <AddressList<0x602BE7, H_CALL>, PRIORITY_AFTER, ArgPick2N<CPad*, 0, int, 1>, void(CPad*, int)> addToPCCheatString;
#endif
		addToPCCheatString += [] (CPad* pad, int) {
			if (!strncmp(cheatString, pad->KeyBoardCheatString, 11)) {
				bobbleHeads = bobbleHeads == false;
				CHud::SetHelpMessage(TheText.Get(bobbleHeads ? "CHEATON" : "CHEATOF"), true);
			}
		};

#ifdef GTA3
		CdeclEvent <AddressList<0x4CFE12, H_CALL>, PRIORITY_AFTER, ArgPickN<CPed*, 0>, void(CPed*)> onPreRender;
#else
		CdeclEvent <AddressList<0x4FE505, H_CALL>, PRIORITY_AFTER, ArgPickN<CPed*, 0>, void(CPed*)> onPreRender;
#endif
		onPreRender += [](CPed* ped) {
			ProcessBobbleHead(ped);
		};
	}
} bobbleHead;

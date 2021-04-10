#include "global.h"
#include "main.h"

#include <stdio.h>

#define STUB_FUNC(func) func { puts("function \"" #func "\" is a stub"); }
#define STUB_FUNC_QUIET(func) func {}

STUB_FUNC_QUIET(bool8 HandleLinkConnection())
STUB_FUNC_QUIET(void Task_InitUnionRoom())
STUB_FUNC(int MultiBoot(struct MultiBootParam *mp))
STUB_FUNC(void RegisterRamReset(u32 resetFlags))
STUB_FUNC(void IntrMain())
STUB_FUNC(void GameCubeMultiBoot_Hash())
STUB_FUNC_QUIET(void GameCubeMultiBoot_Main())
STUB_FUNC(void GameCubeMultiBoot_ExecuteProgram())
STUB_FUNC(void GameCubeMultiBoot_Init())
STUB_FUNC(void GameCubeMultiBoot_HandleSerialInterrupt())
STUB_FUNC(void GameCubeMultiBoot_Quit())
STUB_FUNC(void rfu_initializeAPI())
STUB_FUNC(void InitRFUAPI(void))
/*STUB_FUNC(void rfu_STC_clearAPIVariables())
STUB_FUNC(void rfu_REQ_PARENT_resumeRetransmitAndChange())
STUB_FUNC(void rfu_UNI_PARENT_getDRAC_ACK())
STUB_FUNC(void rfu_setTimerInterrupt())
STUB_FUNC(void rfu_getSTWIRecvBuffer())
STUB_FUNC(void rfu_setMSCCallback())
STUB_FUNC(void rfu_setREQCallback())
STUB_FUNC(void rfu_enableREQCallback())
STUB_FUNC(void rfu_STC_REQ_callback())
STUB_FUNC(void rfu_CB_defaultCallback())
STUB_FUNC(void rfu_waitREQComplete())
STUB_FUNC(void rfu_REQ_RFUStatus())
STUB_FUNC(void rfu_getRFUStatus())
STUB_FUNC(void rfu_MBOOT_CHILD_inheritanceLinkStatus())
STUB_FUNC(void rfu_REQ_stopMode())
STUB_FUNC(void rfu_CB_stopMode())
STUB_FUNC(void rfu_REQBN_softReset_and_checkID())
STUB_FUNC(void rfu_REQ_reset())
STUB_FUNC(void rfu_CB_reset())
STUB_FUNC(void rfu_REQ_configSystem())
STUB_FUNC(void rfu_REQ_configGameData())
STUB_FUNC(void rfu_CB_configGameData())
STUB_FUNC(void rfu_REQ_startSearchChild())
STUB_FUNC(void rfu_CB_startSearchChild())
STUB_FUNC(void rfu_STC_clearLinkStatus())
STUB_FUNC(void rfu_REQ_pollSearchChild())
STUB_FUNC(void rfu_REQ_endSearchChild())
STUB_FUNC(void rfu_CB_pollAndEndSearchChild())
STUB_FUNC(void rfu_STC_readChildList())
STUB_FUNC(void rfu_REQ_startSearchParent())
STUB_FUNC(void rfu_CB_startSearchParent())
STUB_FUNC(void rfu_REQ_pollSearchParent())
STUB_FUNC(void rfu_CB_pollSearchParent())
STUB_FUNC(void rfu_REQ_endSearchParent())
STUB_FUNC(void rfu_STC_readParentCandidateList())
STUB_FUNC(void rfu_REQ_startConnectParent())
STUB_FUNC(void rfu_REQ_pollConnectParent())
STUB_FUNC(void rfu_CB_pollConnectParent())
STUB_FUNC(void rfu_getConnectParentStatus())
STUB_FUNC(void rfu_REQ_endConnectParent())
STUB_FUNC(void rfu_syncVBlank())
STUB_FUNC(void rfu_REQBN_watchLink())
STUB_FUNC(void rfu_STC_removeLinkData())
STUB_FUNC(void rfu_REQ_disconnect())
STUB_FUNC(void rfu_CB_disconnect())
STUB_FUNC(void rfu_REQ_CHILD_startConnectRecovery())
STUB_FUNC(void rfu_REQ_CHILD_pollConnectRecovery())
STUB_FUNC(void rfu_CB_CHILD_pollConnectRecovery())
STUB_FUNC(void rfu_CHILD_getConnectRecoveryStatus())
STUB_FUNC(void rfu_REQ_CHILD_endConnectRecovery())
STUB_FUNC(void rfu_STC_fastCopy())
STUB_FUNC(void rfu_REQ_changeMasterSlave())
STUB_FUNC(void rfu_getMasterSlave())
STUB_FUNC(void rfu_clearAllSlot())
STUB_FUNC(void rfu_STC_releaseFrame())
STUB_FUNC(void rfu_clearSlot())
STUB_FUNC(void rfu_setRecvBuffer())
STUB_FUNC(void rfu_NI_setSendData())
STUB_FUNC(void rfu_UNI_setSendData())
STUB_FUNC(void rfu_NI_CHILD_setSendGameName())
STUB_FUNC(void rfu_STC_setSendData_org())
STUB_FUNC(void rfu_changeSendTarget())
STUB_FUNC(void rfu_NI_stopReceivingData())
STUB_FUNC(void rfu_UNI_changeAndReadySendData())
STUB_FUNC(void rfu_UNI_readySendData())
STUB_FUNC(void rfu_UNI_clearRecvNewDataFlag())
STUB_FUNC(void rfu_REQ_sendData())
STUB_FUNC(void rfu_CB_sendData())
STUB_FUNC(void rfu_CB_sendData2())
STUB_FUNC(void rfu_CB_sendData3())
STUB_FUNC(void rfu_constructSendLLFrame())
STUB_FUNC(void rfu_STC_NI_constructLLSF())
STUB_FUNC(void rfu_STC_UNI_constructLLSF())
STUB_FUNC(void rfu_REQ_recvData())
STUB_FUNC(void rfu_CB_recvData())
STUB_FUNC(void rfu_STC_PARENT_analyzeRecvPacket())
STUB_FUNC(void rfu_STC_CHILD_analyzeRecvPacket())
STUB_FUNC(void rfu_STC_analyzeLLSF())
STUB_FUNC(void rfu_STC_UNI_receive())
STUB_FUNC(void rfu_STC_NI_receive_Sender())
STUB_FUNC(void rfu_STC_NI_receive_Receiver())
STUB_FUNC(void rfu_STC_NI_initSlot_asRecvControllData())
STUB_FUNC(void rfu_STC_NI_initSlot_asRecvDataEntity())
STUB_FUNC(void rfu_NI_checkCommFailCounter())
STUB_FUNC(void rfu_REQ_noise())
STUB_FUNC(void AgbRFU_checkID())

STUB_FUNC(u32 VerifyFlashSectorNBytes(u16 sectorNum, u8 *src, u32 n))
STUB_FUNC(u32 VerifyFlashSector(u16 sectorNum, u8 *src))
STUB_FUNC(void Sio32IDInit())
STUB_FUNC(void Sio32IDMain())
STUB_FUNC(void Sio32IDIntr())
STUB_FUNC(void umul3232H32())
STUB_FUNC_QUIET(void SoundMain())
STUB_FUNC(void SoundMainRAM())
STUB_FUNC(void SoundMainBTM())
STUB_FUNC(void RealClearChain())
STUB_FUNC(void ply_fine())
STUB_FUNC(void MPlayJumpTableCopy())
STUB_FUNC(void ply_goto())
STUB_FUNC(void ply_patt())
STUB_FUNC(void ply_pend())
STUB_FUNC(void ply_rept())
STUB_FUNC(void ply_prio())
STUB_FUNC(void ply_tempo())
STUB_FUNC(void ply_keysh())
STUB_FUNC(void ply_voice())
STUB_FUNC(void ply_vol())
STUB_FUNC(void ply_pan())
STUB_FUNC(void ply_bend())
STUB_FUNC(void ply_bendr())
STUB_FUNC(void ply_lfodl())
STUB_FUNC(void ply_modt())
STUB_FUNC(void ply_tune())
STUB_FUNC(void ply_port())
STUB_FUNC(void m4aSoundVSync())
STUB_FUNC(void MPlayMain())
STUB_FUNC(void TrackStop())
STUB_FUNC(void ChnVolSetAsm())
STUB_FUNC(void ply_note())
STUB_FUNC(void ply_endtie())
STUB_FUNC(void clear_modM())
STUB_FUNC(void ld_r3_tp_adr_i())
STUB_FUNC(void ply_lfos())
STUB_FUNC(void ply_mod())
STUB_FUNC(u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust))
STUB_FUNC(void UnusedDummyFunc(void))
STUB_FUNC(void MPlayContinue(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void MPlayFadeOut(struct MusicPlayerInfo *mplayInfo, u16 speed))
STUB_FUNC(void m4aSoundInit(void))
STUB_FUNC_QUIET(void m4aSoundMain(void))
STUB_FUNC(void m4aSongNumStart(u16 n))
STUB_FUNC(void m4aSongNumStartOrChange(u16 n))
STUB_FUNC(void m4aSongNumStartOrContinue(u16 n))
STUB_FUNC(void m4aSongNumStop(u16 n))
STUB_FUNC(void m4aSongNumContinue(u16 n))
STUB_FUNC(void m4aMPlayAllStop(void))
STUB_FUNC(void m4aMPlayContinue(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void m4aMPlayAllContinue(void))
STUB_FUNC(void m4aMPlayFadeOut(struct MusicPlayerInfo *mplayInfo, u16 speed))
STUB_FUNC(void m4aMPlayFadeOutTemporarily(struct MusicPlayerInfo *mplayInfo, u16 speed))
STUB_FUNC(void m4aMPlayFadeIn(struct MusicPlayerInfo *mplayInfo, u16 speed))
STUB_FUNC(void m4aMPlayImmInit(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void MPlayExtender(struct CgbChannel *cgbChans))
STUB_FUNC(void MusicPlayerJumpTableCopy(void))
STUB_FUNC(void ClearChain(void *x))
STUB_FUNC(void Clear64byte(void *x))
STUB_FUNC(void SoundInit(struct SoundInfo *soundInfo))
STUB_FUNC(void SampleFreqSet(u32 freq))
STUB_FUNC(void m4aSoundMode(u32 mode))
STUB_FUNC(void SoundClear(void))
STUB_FUNC(void m4aSoundVSyncOff(void))
STUB_FUNC(void m4aSoundVSyncOn(void))
STUB_FUNC(void MPlayOpen(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *tracks, u8 trackCount))
STUB_FUNC(void MPlayStart(struct MusicPlayerInfo *mplayInfo, struct SongHeader *songHeader))
STUB_FUNC(void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void FadeOutBody(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void TrkVolPitSet(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(u32 MidiKeyToCgbFreq(u8 chanNum, u8 key, u8 fineAdjust))
STUB_FUNC(void CgbOscOff(u8 chanNum))
STUB_FUNC(void CgbModVol(struct CgbChannel *chan))
STUB_FUNC(void CgbSound(void))
STUB_FUNC(void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, u16 tempo))
STUB_FUNC(void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u16 volume))
STUB_FUNC(void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s16 pitch))
STUB_FUNC(void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s8 pan))
STUB_FUNC(void ClearModM(struct MusicPlayerTrack *track))
STUB_FUNC(void m4aMPlayModDepthSet(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u8 modDepth))
STUB_FUNC(void m4aMPlayLFOSpeedSet(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u8 lfoSpeed))
STUB_FUNC(void ply_memacc(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xcmd(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xxx(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xwave(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xtype(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xatta(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xdeca(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xsust(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xrele(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xiecv(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xiecl(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xleng(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xswee(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xcmd_0C(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void ply_xcmd_0D(struct MusicPlayerInfo *mplayInfo, struct MusicPlayerTrack *track))
STUB_FUNC(void DummyFunc(void))
*/
STUB_FUNC(struct MusicPlayerInfo *SetPokemonCryTone(struct ToneData *tone))
STUB_FUNC(void SetPokemonCryVolume(u8 val))
STUB_FUNC(void SetPokemonCryPanpot(s8 val))
STUB_FUNC(void SetPokemonCryPitch(s16 val))
STUB_FUNC(void SetPokemonCryLength(u16 val))
STUB_FUNC(void SetPokemonCryRelease(u8 val))
STUB_FUNC(void SetPokemonCryProgress(u32 val))
STUB_FUNC(void SetPokemonCryChorus(s8 val))
STUB_FUNC(void SetPokemonCryStereo(u32 val))
STUB_FUNC(void SetPokemonCryPriority(u8 val))
STUB_FUNC(void IsPokemonCryPlaying(struct MusicPlayerInfo *mplayInfo))
STUB_FUNC(void MultiBootInit(struct MultiBootParam *mp))
STUB_FUNC(int MultiBootMain(struct MultiBootParam *mp))
STUB_FUNC(void MultiBootStartProbe(struct MultiBootParam *mp))
STUB_FUNC(void MultiBootStartMaster(struct MultiBootParam *mp, const u8 *srcp, int length, u8 palette_color, s8 palette_speed))
STUB_FUNC(int MultiBootCheckComplete(struct MultiBootParam *mp))
//STUB_FUNC(IntrFunc IntrSIO32(void))

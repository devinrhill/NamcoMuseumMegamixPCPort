set(MEGAMIX_SOURCES
  src/cJetNode.cpp
  src/cJetTask.cpp
  src/cSysTaskUser.cpp
  src/JetNodeUtil.cpp
  src/CommonProc.cpp
  src/TestTask.cpp
  src/TestTask2.cpp
  src/WiiPad.cpp
  src/WiiSystem.cpp
  src/wii/Archive.cpp
  src/gfx/TextureCache.cpp
  src/wii/Brres.cpp
  src/wii/Mdl0.cpp
  src/wii/Anim.cpp
  src/gfx/ModelDraw.cpp
  src/wii/Ptk.cpp
  src/gfx/PtkSim.cpp
  src/wii/Nand.cpp
  src/kingdom/cKingdomSave.cpp
  src/GamePad.cpp
  src/cJetAnita.cpp
  src/jet/JetUtil.cpp
  src/kingdom/cKingdomAnita.cpp
  src/kingdom/cKingdomAnitaVector.cpp
  src/kingdom/cTaskFade.cpp
  src/kingdom/cKingdomFade.cpp
  src/Constants.cpp
  src/CustomKingdomUtil.cpp
  src/kingdom/cTaskKingdom.cpp
  src/cSysCoreStructure.cpp
  src/KingdomGlobal.cpp
  src/kingdom/cKingdomSys.cpp
  src/kingdom/cAppKingdom.cpp
  src/kingdom/KingdomJob.cpp
  src/kingdom/cAppSaveLoad.cpp
  src/kingdom/boot/cAppBoot.cpp
  src/kingdom/menu/cAppTitle.cpp
  src/kingdom/menu/cAppAttract.cpp
  src/kingdom/cAppDebugMenu.cpp
  src/UnityUiStubs.cpp
  src/gfx/DebugText.cpp
  src/gfx/FontWriter.cpp
  src/kingdom/MsgTable.cpp
  src/kingdom/cTaskCursor.cpp
  src/kingdom/cTaskLensFlare.cpp
  src/wii/Rfnt.cpp
  src/wii/Dol.cpp
  src/wii/Thp.cpp
  src/snd/Strm.cpp
  src/snd/Archive.cpp
  src/snd/Mixer.cpp
  src/wii/ThpPlayer.cpp
  src/kingdom/menu/TitleScene.cpp
  src/kingdom/ResourceDatabase.cpp
  src/InGamePadManager.cpp
  src/SoundManager.cpp
  src/cJetCSV.cpp
  src/kingdom/cKingdomCSV.cpp
  src/kingdom/cTaskAnita.cpp
  src/kingdom/Constants.cpp
  src/SSaveDataPacnRoll.cpp
  src/kingdom/game/cAppGameBaseMenu.cpp
  src/kingdom/game/cTaskConsMenuBase.cpp
  src/kingdom/game/cTaskConsMenuManBase.cpp
  src/kingdom/game/menu/cTaskConsMenuTutorialBase.cpp
  src/kingdom/game/g03/cG03PacnRollSys.cpp
  src/kingdom/game/g03/cSaveLoadMan.cpp
  src/kingdom/game/g03/cAppG03PacnRollMenu.cpp
  src/kingdom/game/g03/cAppPacnRollDebugMenu.cpp
  src/kingdom/game/g03/cAppG03PacnRollNunchuk.cpp
  src/kingdom/game/menu/cAppGameBaseNunchuk.cpp
  src/kingdom/game/g03/cTaskMenuPacman.cpp
  src/kingdom/game/g03/cTaskConsMenuMan.cpp
  src/kingdom/game/g03/cTaskConsMenuStage.cpp
  src/kingdom/game/g03/cTaskConsMenuBar.cpp
  src/kingdom/game/g03/cTaskConsMenuWorld.cpp
  src/kingdom/game/g03/cTaskConsMenuMap.cpp
  src/kingdom/game/g03/cTaskConsMenuArrow.cpp
  src/kingdom/game/g03/cTaskConsMenuBack.cpp
  src/kingdom/game/g03/cTaskConsMenuBg.cpp
  src/kingdom/game/g03/cTaskConsMenuCurtain.cpp
  src/kingdom/game/g03/cTaskConsMenuRule.cpp
  src/kingdom/game/g03/cTaskConsMenuTutorial.cpp
  src/AABB.cpp
  src/LINE3.cpp
  src/RAY3.cpp
  src/SEGMENT3.cpp
  src/SPHERE.cpp
  src/PLANE.cpp
  src/DummyNW4R.cpp
  src/CullingStageModel.cpp
  src/ResMdl.cpp
  src/ResNode.cpp
  src/ResUserData.cpp
  src/ResUserDataItem.cpp
  src/Extentions.cpp
  src/cJetPtkHnd.cpp
  src/RMDLUserDataParam.cpp
  src/kingdom/cKingdomModelBase.cpp
  src/kingdom/cKingdomModel.cpp
  src/kingdom/cTaskModelBase.cpp
  src/kingdom/cTaskModel.cpp
  src/kingdom/cTaskPushBack.cpp
  src/kingdom/cTaskPBModel.cpp
  src/kingdom/cTaskPBPtkMan.cpp
  src/kingdom/cKingdomPtkMan.cpp
  src/kingdom/cKingdomPtkHnd.cpp
  src/kingdom/NativeScene.cpp
  src/PtkStubs.cpp
  src/kingdom/game/cTaskStaffRollAnita.cpp
  src/kingdom/game/cTaskStaffRollModel.cpp
  src/kingdom/ResHandle.cpp
  src/kingdom/game/cTaskGameBasePause.cpp
  src/kingdom/game/cAppGameBaseGameOver.cpp
  src/kingdom/game/cAppGameBaseStaffRoll.cpp
  src/kingdom/game/cTaskConsRetry.cpp
  src/kingdom/game/g03/Constants.cpp
  src/kingdom/game/g03/cAppG03PacnRollGame.cpp
  src/kingdom/game/g03/cAppMenu.cpp
  src/kingdom/game/g03/cAppG03PacnRollGameOver.cpp
  src/kingdom/game/g03/cAppG03PacnRollStaffRoll.cpp
  src/kingdom/game/g03/cCameraParam.cpp
  src/kingdom/game/g03/cFaceList.cpp
  src/kingdom/game/g03/cGhostCommonParam.cpp
  src/kingdom/game/g03/cGhostParam.cpp
  src/kingdom/game/g03/cGolvisAIArea.cpp
  src/kingdom/game/g03/cGondolaEffect.cpp
  src/kingdom/game/g03/cHitChar.cpp
  src/kingdom/game/g03/cItem.cpp
  src/kingdom/game/g03/cTaskBg.cpp
  src/kingdom/game/g03/cTaskBgBase.cpp
  src/kingdom/game/g03/cTaskBgBox.cpp
  src/kingdom/game/g03/cTaskBgGate.cpp
  src/kingdom/game/g03/cTaskBgGimmick.cpp
  src/kingdom/game/g03/cTaskBgGimmickAddinSnd.cpp
  src/kingdom/game/g03/cTaskBgGimmickArrow.cpp
  src/kingdom/game/g03/cTaskBgGimmickElevator.cpp
  src/kingdom/game/g03/cTaskBgGimmickFall.cpp
  src/kingdom/game/g03/cTaskBgGimmickFallSw.cpp
  src/kingdom/game/g03/cTaskBgGimmickFloorEvent.cpp
  src/kingdom/game/g03/cTaskBgGimmickGondola.cpp
  src/kingdom/game/g03/cTaskBgGimmickIronBall.cpp
  src/kingdom/game/g03/cTaskBgGimmickMerryGoRound.cpp
  src/kingdom/game/g03/cTaskBgGimmickPillar.cpp
  src/kingdom/game/g03/cTaskBgGimmickPistonMan.cpp
  src/kingdom/game/g03/cTaskBgGimmickSeesaw.cpp
  src/kingdom/game/g03/cTaskBgGimmickSwitch.cpp
  src/kingdom/game/g03/cTaskBgGimmickTruck.cpp
  src/kingdom/game/g03/cTaskBgGimmickWall.cpp
  src/kingdom/game/g03/cTaskBgHit.cpp
  src/kingdom/game/g03/cTaskBox.cpp
  src/kingdom/game/g03/cTaskCamera.cpp
  src/kingdom/game/g03/cTaskCameraSet.cpp
  src/kingdom/game/g03/cTaskChar.cpp
  src/kingdom/game/g03/cTaskCharAreaGhost.cpp
  src/kingdom/game/g03/cTaskCharBEye.cpp
  src/kingdom/game/g03/cTaskCharEnemy.cpp
  src/kingdom/game/g03/cTaskCharGear1.cpp
  src/kingdom/game/g03/cTaskCharGear2.cpp
  src/kingdom/game/g03/cTaskCharGhost.cpp
  src/kingdom/game/g03/cTaskCharGhostBlinky.cpp
  src/kingdom/game/g03/cTaskCharGhostBoat.cpp
  src/kingdom/game/g03/cTaskCharGhostBomb.cpp
  src/kingdom/game/g03/cTaskCharGhostCar.cpp
  src/kingdom/game/g03/cTaskCharGhostClyde.cpp
  src/kingdom/game/g03/cTaskCharGhostHover.cpp
  src/kingdom/game/g03/cTaskCharGhostInky.cpp
  src/kingdom/game/g03/cTaskCharGhostPinky.cpp
  src/kingdom/game/g03/cTaskCharGhostPlane.cpp
  src/kingdom/game/g03/cTaskCharGhostUfo.cpp
  src/kingdom/game/g03/cTaskCharGolvis.cpp
  src/kingdom/game/g03/cTaskCharGolvis01.cpp
  src/kingdom/game/g03/cTaskCharGolvis02.cpp
  src/kingdom/game/g03/cTaskCharGolvis03.cpp
  src/kingdom/game/g03/cTaskCharGolvis04.cpp
  src/kingdom/game/g03/cTaskCharGolvis05.cpp
  src/kingdom/game/g03/cTaskCharNonRideGhost.cpp
  src/kingdom/game/g03/cTaskCharPacman.cpp
  src/kingdom/game/g03/cTaskCharRouteGhost.cpp
  src/kingdom/game/g03/cTaskCollision.cpp
  src/kingdom/game/g03/cTaskConsBomCursor.cpp
  src/kingdom/game/g03/cTaskConsButtCursor.cpp
  src/kingdom/game/g03/cTaskConsCheckPoint.cpp
  src/kingdom/game/g03/cTaskConsCombo.cpp
  src/kingdom/game/g03/cTaskConsCookieWarning.cpp
  src/kingdom/game/g03/cTaskConsEnemyLife.cpp
  src/kingdom/game/g03/cTaskConsGate.cpp
  src/kingdom/game/g03/cTaskConsNotCameraCtrl.cpp
  src/kingdom/game/g03/cTaskConsPCookie.cpp
  src/kingdom/game/g03/cTaskConsoleMan.cpp
  src/kingdom/game/g03/cTaskEffect.cpp
  src/kingdom/game/g03/cTaskFallArea.cpp
  src/kingdom/game/g03/cTaskGate.cpp
  src/kingdom/game/g03/cTaskGateCookie.cpp
  src/kingdom/game/g03/cTaskGhostArea.cpp
  src/kingdom/game/g03/cTaskGhostMan.cpp
  src/kingdom/game/g03/cTaskGhostRoute.cpp
  src/kingdom/game/g03/cTaskGoal.cpp
  src/kingdom/game/g03/cTaskItem.cpp
  src/kingdom/game/g03/cTaskLandAttack.cpp
  src/kingdom/game/g03/cTaskLight.cpp
  src/kingdom/game/g03/cTaskModelEx.cpp
  src/kingdom/game/g03/cTaskPacFade.cpp
  src/kingdom/game/g03/cTaskPause.cpp
  src/kingdom/game/g03/cTaskPoint.cpp
  src/kingdom/game/g03/cTaskShadowCamera.cpp
  src/kingdom/game/g03/cTaskStage.cpp
  src/kingdom/game/g03/cTaskWater.cpp
  src/kingdom/game/g03/cTaskWind.cpp
  src/kingdom/game/g03/sCheckPoint.cpp
  src/kingdom/game/g03/cTaskConsoleBase.cpp
  src/kingdom/game/g03/cTaskConsNum.cpp
  src/kingdom/game/g03/cTaskConsStart.cpp
  src/kingdom/game/g03/cTaskConsStageName.cpp
  src/kingdom/game/g03/cTaskConsClear.cpp
  src/kingdom/game/g03/cTaskConsCookie.cpp
  src/kingdom/game/g03/cTaskConsHaveCookie.cpp
  src/kingdom/game/g03/cTaskConsStock.cpp
  src/kingdom/game/g03/cTaskConsTimer.cpp
  src/kingdom/game/g03/cTaskConsPacLife.cpp
  src/kingdom/game/g03/cTaskConsGhost.cpp
)

# Pac-Motos (g07): every converted file (syntax check clean: tools/g07check.sh)
file(GLOB G07_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g07/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G07_SOURCES})

# Galaga Remix (g01): hand-ported from main.dol (no C# exists); the raw Ghidra output lives in reversing/dol/g01_ghidra
file(GLOB G01_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g01/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G01_SOURCES})

# Rally-X Remix (g02): hand-ported from main.dol (no C# exists)
file(GLOB G02_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g02/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G02_SOURCES})

# g00 dummy game (debug-text placeholder): hand-ported from main.dol (no C# exists)
file(GLOB G00_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g00/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G00_SOURCES})

# Gator Panic Remix (g04): hand-ported from main.dol (no C# exists)
file(GLOB G04_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g04/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G04_SOURCES})

# Grobda Remix (g11): hand-ported from main.dol (no C# exists)
file(GLOB G11_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/g11/*.cpp)
list(APPEND MEGAMIX_SOURCES ${G11_SOURCES})

# Unity UI / engine pieces the Pac-Motos code needs (stubs and converted C#)
list(APPEND MEGAMIX_SOURCES
  src/SSaveDataPacMotos.cpp
  src/KingdomAnimManager.cpp
  src/kingdom/cKingdomMapoo.cpp
  src/kingdom/sMatchingData.cpp
  src/kingdom/game/menu/cAppGameBaseMenuMatching.cpp
  src/MotosUIStubs.cpp
  src/DObjParam.cpp
  src/kingdom/game/menu/cTaskConsError.cpp src/kingdom/game/menu/cTaskSceneBase.cpp src/kingdom/game/menu/cTaskConsBg.cpp
  src/kingdom/game/menu/cParamMessage.cpp src/kingdom/game/menu/cTaskScenePerson.cpp src/kingdom/game/menu/cTaskSceneConfirm.cpp
  src/kingdom/game/menu/cTaskSceneAdd.cpp src/kingdom/game/menu/cTaskConsInfoWnd.cpp
)

# the shared menu / matching classes of the games (kingdom::game::menu): the Pac-Motos matching apps use them
file(GLOB GAME_MENU_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/kingdom/game/menu/*.cpp)
list(APPEND MEGAMIX_SOURCES ${GAME_MENU_SOURCES})
list(REMOVE_DUPLICATES MEGAMIX_SOURCES)

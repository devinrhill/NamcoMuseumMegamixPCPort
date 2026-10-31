#pragma once
// Native boot: the same job-driven startup the Wii executable runs from main() (0x80121b78).

class cJetNode;

namespace boot
{
void Init();      // SysCore + job table + first job
void SetG03StandIn(bool on);   // --g03-standin: run the old native Pac-N-Roll stand-in (cAppG03PacnRollPlay) instead of the converted cAppG03PacnRollGame
void SetG03Round(int round);   // --round: Pac-N-Roll stage 0..25
constexpr unsigned NO_JOB = 0xFFFFFFFFu;   // "not given": job 0 is a real job (the root debug menu)
void SetEntryJob(unsigned job, unsigned nextJob = NO_JOB); // test hook: run `job` instead of cAppBoot; nextJob = where a transition app hands off
void Frame();     // one pass of cSysCoreStructure::Process()
bool JobAvailable(unsigned job); // the port has an init for this job number (index = job >> 20)
cJetNode* Root(); // root of the task tree (for debug printing)
}

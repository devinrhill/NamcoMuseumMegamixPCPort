# Galaga Remix stages: how they work

How a Galaga Remix stage is put together, for writing stages with the dev window's **Galaga editor** tab (`--devui`, see
[OPTIONS.md](OPTIONS.md)). Everything here comes from the native port's game code (`cTaskScene`, `cTaskEnemy`) and stage 1's own data.
The few parts not yet seen in a run are marked **UNVERIFIED**.

## Overview

A stage is a camera flight on rails, a script that paces it and starts enemies, and a set of flight paths those enemies follow.

- **The course** is a recorded camera flight (an SCN0 animation). The stage is drawn around it.
- **The scn file** is a list of records. *Course commands* set how fast the camera moves; *spawns* start enemies.
- **Orbits** (.emy files) are enemy flight paths. The stage's `EnemyOrbit_sN.csv` numbers them; a spawn's "orbit" is a row in that table.

There is one scn per player count (`scn_s1p1.scn` .. `scn_s1p4.scn`); the editor works on the single-player one.

## Course time and speed

Course time is how far along the camera flight you are; course speed is how much course time passes per game frame (60 per second).

- **Course time** = the frame of the camera animation. The editor's playhead shows it.
- **Course speed** multiplies a base speed of 1.0. Stage 1 runs at 0.1 to 0.28: at 0.24, 100 course time takes about 7 seconds.
  Speed 0 stops the camera.
- **Pac-Man speed** is a second speed each command sets the same way. Stage 1 mostly sets it equal to the course speed.
- Enemies do not use course speed: their orbits count plain game frames (see [Orbits](#orbits)).

## Course commands

The course always runs from one command to the next in list order, and the command you are in decides when you move on.

| Command | What it does | Moves on when |
| --- | --- | --- |
| ADVANCE | Keeps its own speed | Course time reaches the next command's time |
| RAMP | Changes speed smoothly from its own value to the next command's, over the stretch between them | Course time reaches the next command's time |
| WAIT | Runs at its own speed (give it 0 to stand still) | Its "delay" in game frames has passed |
| GATE | Holds the course until certain enemies are gone (see [Groups and gates](#groups-and-gates)) | Its groups are cleared |

- A RAMP speed between -1 and 0.01 is raised to 0.01, so a ramp never quite stops.
- **The last command is the end of the course.** Once it is reached there is no next command and the stage ends.
- Only WAIT and GATE read "delay". The delays on stage 1's RAMPs (182, 142, ...) are ignored.

Parameter 2 flags on a command (they take effect when the course enters it):

- bit 0: a mist effect
- bit 1: a stone effect
- bit 2: Pac-Man's jumps are off until the next command

## Spawns

A spawn belongs to the stretch between the two commands around it in the list, and fires once while the course is in that stretch.

- **When it fires depends on whether the course is moving:**
  - Moving: when course time passes the spawn's course time.
  - Standing still (speed 0, as in a WAIT or a stopped GATE): when the frames counted since that command started pass the spawn's delay.
- **A spawn whose time falls outside its own stretch never fires.** The Check page lists these as "never".
- **Position** is where the orbit starts. It is measured from the point the camera looks at, in the camera's own directions: x sideways,
  y up, z along the view (+z farther away, -z toward the camera). Enemies therefore move with the screen while the course moves. In stage 1
  the formation sits at z 0, enemies coming toward you start near -30, the far ones at +38.
- **Enemy type** picks the model and its life / score / bullet speed (zako, goei, boss, ...).
- **Group** (bits 0-8 of parameter 2): see [Groups and gates](#groups-and-gates).
- **Marker** (bits 9-10): any nonzero value sends a fighter along with the enemy on the same orbit. Probably the captured-fighter
  mechanic (UNVERIFIED). Stage 1 uses it once, on a goei.

## Groups and gates

A gate opens once every group from 0 up to its parameter 1 is cleared, and a group is cleared the moment any one of its enemies leaves.

- **Leaving** = shot down, or reaching the end of its orbit.
- **Cleared groups stay cleared** for the rest of the stage.
- **Group 0 is cleared almost at once** by the first ordinary enemy, so treat it as "no group".
- **A gate with speed 0** stands still: it waits its delay in frames, then checks every frame.
- **A gate with speed above 0** keeps moving; when the course reaches the next command and the groups are not cleared, it rewinds to the
  gate's start and replays that stretch. The spawns in that stretch then fire again on every replay (read from the code; UNVERIFIED in a run).

```
Course enters the gate
  speed 0?  yes -> wait its delay -> groups cleared? -- no:  check again next frame
                                                     \- yes: next command
            no  -> run on to the next command's time -> groups cleared? -- yes: next command
                                                                       \- no:  rewind to the gate's start (its spawns fire again), run on again
```

### Stage 1's formation

1. A WAIT at course time 673 with speed 0 stops the camera.
2. 38 spawns follow, 2 frames apart by delay, each in its own group 2..39.
3. The GATE has parameter 1 = 39, speed 0, delay 1.
4. The enemies fly into place in 30 frames, hold still, and fly off and end at frame 600 (orbit attack61).

So the gate opens when every one of them is shot **or** has flown off, whichever comes first. To force the player to shoot them, end the
orbit with a jump back to an earlier node so it never ends.

## Orbits

An orbit is a list of nodes timed in game frames since the enemy spawned; the enemy moves from node to node and disappears at the last one.

- **Frames count 1 per game frame**, whatever the course speed. They stop while paused or after Pac-Man's miss.
- **Positions are added to the spawn's position**, in the same camera frame.
- **The last node is where the enemy vanishes.** Put it off screen.

| Node type | What happens until the next node's frame |
| --- | --- |
| linear | Moves in a straight line toward the next node |
| hermite | Curves toward the next node, shaped by both nodes' tangents |
| jump | Goes back to node N at once and carries on from its frame: this is how you loop |
| hold | Sits still at this node and sets the fire pattern |

| Facing | The enemy points |
| --- | --- |
| 0 node rotation | Along the view, plus the node's own rotation (interpolated between nodes) |
| 1 face target | At Pac-Man's aim point (the most common on the disc) |
| 2 from camera | Away from the camera |
| 3 to camera | Toward the camera |

### Firing

Only hold nodes fire. On the disc they last 0 or 1 frames, so treat them as "fire here" markers. The fire pattern is the hold node's
parameter:

- 0: an aimed bullet
- 1: a warning flash, then a bullet
- 2: randomly 0 or 1
- 3: a warning flash, then a missile

A longer hold re-arms the pattern every frame, so pattern 0 would fire every frame while bullets are free (read from the code; UNVERIFIED).

## Writing a stage

1. Lay out RAMPs along the camera flight to set the pace. The last command is the end.
2. Place each spawn after the command whose stretch it belongs to, timed by course time.
3. For a formation fight: a WAIT with speed 0, spawns timed by delay in their own groups, then a GATE whose parameter 1 is the highest group.
4. Loop the formation orbits with a jump if the player must shoot them; let them end if the gate should time out.
5. Use **Play to it** to watch any record, and keep the Check page clean.

Not yet seen in a run: spawns re-firing in a moving gate's replay, the marker's fighter, and a long hold firing every frame.

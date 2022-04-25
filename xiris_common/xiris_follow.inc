#EVENT Follow "[#1#] Follow #2#"
#EVENT Follow "<#1#> Follow #2#"
#EVENT Follow "#1# tells you, 'Follow #2#'"
#EVENT Follow "#1# tells the group, 'Follow #2#'"
#EVENT Follow "#1# you tell your party, 'Follow #2#'"

#EVENT Stop	"[#1#] Stop"
#EVENT Stop	"<#1#> Stop"
#EVENT Stop "#1# tells you, 'Stop'"
#EVENT Stop "#1# tells the group, 'Stop'"
#EVENT Stop "#1# you tell your party, 'Stop'"

#EVENT MoveHere "<#1#> Move to #2#"
#EVENT MoveHere "#1# tells you, 'Move to #2#'"
#EVENT MoveHere "#1# tells the group, 'Move to #2#'"
#EVENT MoveHere "#1# you tell your party, 'Move to #2#'"

Sub InitEvents_Following
	/call RegisterBackgroundEvents Events_Following 3
/return

Sub Events_Following
	/doevents Follow
	/doevents Stop
	/doevents MoveHere
/return



Sub xbot_initialize_following
	/call InitEvents_Following
	/cecho \aoxbot_initialize_following
	/declare Following 		bool 	outer FALSE
	/declare FollowTarget 	string 	outer NULL
	/declare MaxResponseDist	int outer 5000
/return


SUB EVENT_Follow(line, ChatSender, eventParams)
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
	
		/declare SpawnToFollow string local ${If[${eventParams.Arg[1, ].Find[/]} || ${eventParams.Arg[1, ].Equal[me]} || ${eventParams.Arg[1, ].Equal[on]},${ChatSender},${eventParams.Arg[1, ]}]}
		/declare FollowType string local ${If[${eventParams.Find[/type|NAV]},NAV,STICK]}
		
		/echo eventParams: ${eventParams}
		/echo FollowType: ${FollowType}
		/if (!${Bool[${SpawnToFollow}]}) /varset SpawnToFollow ${ChatSender}
		
		| -Turn Following on and set the FollowTarget
		/varset Following TRUE
		/varset FollowTarget ${Spawn[pc ${SpawnToFollow}].CleanName}
		
		| -Check Range
		/if (${Spawn[=${FollowTarget}].Distance} > ${MaxResponseDist}) {
			/docommand /${cmdChannel} [${FollowTarget}] is too far away.
		} else {
		
			| -Check LoS.
			/echo LOS probably not needed because Type is NAV: ${FollowType.Equal[NAV]}
			/if (${FollowType.Equal[NAV]}) {
				| -Stop assisting so we don't try to stick to the AssistTarget.	
				/if (${Me.Combat}) /call Event_BackOff
				
				| -Start Following.
				/if (${Verbosity}) /docommand /${cmdChannel} Following ${FollowTarget}.
				/call AquireFollow ${FollowType}
			
			} else {
			
				/if (!${Spawn[=${FollowTarget}].LineOfSight} ) {
					/docommand /${cmdChannel} I cannot see [${FollowTarget}].
				} else {
					| -Stop assisting so we don't try to stick to the AssistTarget.	
					/if (${Me.Combat}) /call Event_BackOff
					
					| -Start Following.
					/if (${Verbosity}) /docommand /${cmdChannel} Following ${FollowTarget}.
					/call AquireFollow ${FollowType}
				}
			} 
		}
		
	}
	/doevents flush Follow
/return



|----------------------------------------------------------------------------------------------------|
|- Engages follow plugins on specified follow targets.												-|
|----------------------------------------------------------------------------------------------------|
|- Uses NetAdvPath if the follow target is one of your bots, or MoveUtils, if its someone elses.	-|		
|----------------------------------------------------------------------------------------------------|
SUB AquireFollow(string _type)

	|------------------------------------------------------------------------------------Check for FollowTarget in zone.
	/if (${SpawnCount[=${FollowTarget}]}) {

		|--------------------------------------------------------------------------------Check range to the FollowTarget
		/if (${SpawnCount[=${FollowTarget} radius ${MaxResponseDist}]}) {

			|----------------------------------------------------------------------------Check that I'm not the follow target.
			/if (${FollowTarget.NotEqual[${Me.CleanName}]}) {

				|------------------------------------------------------------------------Turn off any previous follow plugins.
				/if (${Stick.Active}) 			/squelch /stick off
				|/if (${NetAdvPath.State}==1) 	/squelch /netfollow off
				/if (${Navigation.Active}) 		/squelch /nav stop
				
				/declare followTargetID int local
				
				|--------------------------------------------------------------------Check Target ID
				/if (${SpawnCount[pc =${FollowTarget}]}) {
					/varset followTargetID ${Spawn[pc =${FollowTarget}].ID}
				} else /if (${SpawnCount[npc ${FollowTarget}]}) {
					/varset followTargetID ${Spawn[npc =${FollowTarget}].ID}
				} else /if (${SpawnCount[pet ${FollowTarget}]}) {
					/varset followTargetID ${Spawn[pet =${FollowTarget}].ID}
				}
				| If the FollowTarget declares that you need to use MQ2Nav...
				
				/if (${_type.Equal[NAV]}) /echo we should be navigating
				
				/if (${_type.Equal[NAV]}) {
					/echo using MQ2Nav ${followTargetID}
					|-- We do not need to use LOS for MQ2Nav
					/call TrueTarget ${followTargetID}
					/delay 5s ${Target.ID} == ${followTargetID}
					/squelch /nav target		
				| If the FollowTarget is a NetBot, and you're using NetAdvPath
				|} else /if (${NetBots[${FollowTarget}].ID} && ${NetAdvPath}) {
				|	/echo using MQ2NetAdvPath
				|	| If the FollowTarget is a NetBot, engage MQ2NetAdvPath.
				|	/if (${Spawn[id ${followTargetID}].LineOfSight}) {
				|		/call TrueTarget ${followTargetID}
				|		/squelch /netfollow on
				|	} else /if (${NetAdvPath.State}==2) {
				|		/squelch /netfollow unpause
				|	}
					
				} else {
					/echo using MQ2MoveUtils
					| If MQ2NetAdvPath was not detected, or the FollowTarget was not a NetBot, engage MQ2MoveUtils.
				
					|---------------------------------------------------Check LoS
					/if (${Spawn[id ${followTargetID}].LineOfSight}) {
						/call TrueTarget ${followTargetID}
						/if (${Me.Underwater}) {
							/squelch /stick hold behind 0 uw
						} else {
							/squelch /stick hold behind 10 uw
						}
						
					}
				}

				|-------------------------------------------------------Clear my target
				|/if (${Target.ID} == ${followTargetID}) /squelch /target clear
			}
		}
	}

	/if (!${Defined[distCheck]}) /declare distCheck int local 10
	:movingLoop
	/call RunAllEvents	

	/if (${Following}) {
		/if (${_type.Equal[NAV]} && !${Navigation.Active} && (${Target.Distance3D} > ${distCheck} )) {
			/echo :movingLoop ${_type.Equal[NAV]} && !${Navigation.Active} && (${Target.Distance3D} > ${distCheck})
			/nav target
			/delay 2s
		}
		/goto :movingLoop
	}

/return



|----------------------------------------------------------------------------|
|- Disengages follow on bots who are in the same zone as the ChatSender.	-|
|----------------------------------------------------------------------------|
|- Use, /stop, or, 'stop' in /tell, /goup, or /bc.							-|
|----------------------------------------------------------------------------|

SUB EVENT_Stop(line, ChatSender, eventParams)
	|- Check if I am attacking
	/if (${bln_engaged}) {
		/call targetReset
	}

	| -Check if I'm following.
	/if (${Following}) {
			/echo Stopping
			/if (${Stick.Active}) 			/squelch /stick off
			|/if (${NetAdvPath.State}==1) 	/squelch /netfollow off
			/if (${Navigation.Active}) 		/squelch /nav stop
			/varset FollowTarget NULL
			/varset Following 	 FALSE
	}
	/doevents flush Stop
/return


SUB EVENT_MoveHere(line, ChatSender, eventParams)

	| -Verify the event
	/if (${Defined[ChatSender]}) /call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
	
		| -Check if the ChatSender is me.
		/if (${ChatSender.NotEqual[${Me}]}) {
			| -Set WhereTo.
			/declare moveTo_Name string local
			/declare WhereTo string local ${If[${eventParams.Arg[1, ].Find[/]} || ${eventParams.Arg[1, ].Equal[me]} || ${eventParams.Arg[1, ].Equal[here]},${ChatSender},${eventParams.Arg[1, ]}]}
			
			| -Change WhereTo to a spawnID
			/if (!${Int[${WhereTo}]}) {
				/if (${SpawnCount[pc =${WhereTo}]}) {
					/varset WhereTo ${Spawn[pc =${WhereTo}].ID}
				} else /if (${SpawnCount[pet =${WhereTo}]}) {
					/varset WhereTo ${Spawn[pet =${WhereTo}].ID}
				} else /if (${SpawnCount[npc =${WhereTo}]}) {
					/varset WhereTo ${Spawn[npc =${WhereTo}].ID}
				}
			}
			
			| Convert SpawnID to Loc
			/varset moveTo_Name ${Spawn[id ${WhereTo}].CleanName}
			/declare y int local ${Spawn[id ${WhereTo}].Y}
			/declare x int local ${Spawn[id ${WhereTo}].X}
			/declare z int local ${Spawn[id ${WhereTo}].Z}
			/varset WhereTo ${y},${x},${z}
			/echo WhereTo ${WhereTo}
			
			| -Stop assisting so I don't try to stick to the AssistTarget.	
			/if (${Me.Combat}) /call Event_BackOff
			
			/declare retryTimer timer local 150
			
			:retry_MoveTo
			/call MoveTo -Loc|${WhereTo} uw
			
			| -Check if I have moved to the Target.
			/call Triangulate_Distance ${WhereTo} ${Me.Loc.Replace[ ,]}
			/if (${Macro.Return} > 10) {
				/if  (${retryTimer}) {
					/goto :retry_MoveTo
				} else {
					/echo |- EVENT_MoveHere -| Failed to move to the specified target.
				}
			} 
			| -If I'm following, reaquire follow.
			/if (${Following}) {
				/if (${moveTo_Name.Equal[${FollowTarget}]}) {
					/call AquireFollow
				} else {
					/call EVENT_Stop "<${Me.CleanName}> Stop" ${Me.CleanName}
				}
			}
		}
	}
	/doevents flush MoveHere
/return


|----------------------------------------------------------------------------------------------------|
|- MoveTo																							-|
|----------------------------------------------------------------------------------------------------|
|- Attempts to utilize MQ2MoveUtils /moveto function, to move to a given destination.				-|
|----------------------------------------------------------------------------------------------------|
|- @param1 destination: Destinations can be provided as an ID, or loc, using the following syntax:	-|
|-																									-|
|- Move to a Spawn ID use, -SpawnID|#																-|
|-		Ex.  /call MoveTo -SpawnID|${Target.ID}														-|
|-																									-|
|- Move to a Location use, -Loc|Y,X																	-|
|-		Ex.  /call MoveTo -Loc|${Target.Y},${Target.X}												-|
|----------------------------------------------------------------------------------------------------|
SUB MoveTo(destinationInfo, int distCheck)
/if (${Debug}) /echo |- MoveTo ==>

	/declare X_Loc  float local
	/declare Y_Loc  float local
	/declare Z_Loc  float local
	/declare dist2D	float local 0
	/declare dist3D float local 0
	/declare distZ  float local 0
	/declare zDir   string local UP

	
	/if (!${Defined[distCheck]}) /declare distCheck int local 10
	/varset distCheck 10

|---------------------------------------------------------Set MoveTo variables per destinationType.
	/if (${destinationInfo.Find[-SpawnID]}) {
		/echo ~~~setting to spawn target
		/call TrueTarget ${destinationInfo.Arg[2,|]}
		/varset Y_Loc ${Target.Y}
		/varset X_Loc ${Target.X}
		/varset Z_Loc ${Target.Z}
		
	} else /if (${destinationInfo.Find[-Loc]}) {
		/echo ~~~setting to dest info
		/varset Y_Loc ${destinationInfo.Arg[2,|].Arg[1,,]}
		/varset X_Loc ${destinationInfo.Arg[2,|].Arg[2,,]}
		/varset Z_Loc ${destinationInfo.Arg[2,|].Arg[3,,]}

	} else {
		/echo [${destinationInfo}] does not specify a valid destinationType.  Use -SpawnID|#, or, -Loc|Y,X,Z to note your intended destinationType.
	}
	
	|-- Calculate the start distance
	/varcalc dist2D ${Math.Distance[${Me.Y},${Me.X}:${Y_Loc},${X_Loc}]}
	/varcalc dist3D ${Math.Distance[${Me.Y},${Me.X},${Me.Z}:${Y_Loc},${X_Loc},${Z_Loc}]}

	/echo dist start: 2D:${dist2D} 3D:${dist3D} Required: ${distCheck}
	|-- Check if target is too far.
	
	/if (${Int[${dist3D}]} > ${MaxResponseDist}) {
		/echo [${Spawn[id ${destination}].CleanName}] is too far away.
	} else {

	|-- Check if target is already close enough
		/if (${Int[${dist3D}]} < 10) {
			/if (${Debug}) /echo I am already at the destination.
		} else {

		| Declare a timer based on how far you have to move (1s for every 50 units + 3).
			/declare retryTimer timer local ${Math.Calc[${Macro.Return} / 50 + 3].Int}s
			
		| Engage MQ2MoveUtils.
			:retry_MoveToLoc
			/if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /interrupt
			/moveto loc ${Y_Loc} ${X_Loc} ${Z_Loc} uw
			/delay 10 ${Me.Moving} || ${MoveTo.Moving}
			
			:movingTo_Loop
			
			| Debug Echo
			/if (${Debug} && !${spamTimer_MTLoop}) {
				/echo |- MoveToSpawn -| :MoveTo_Loop
				/call createTimer spamTimer_MTLoop 50
			}

			/call RunRaidEvents	
			|-- Calculate the current distance
			/varcalc dist2D ${Math.Distance[${Me.Y},${Me.X}:${Y_Loc},${X_Loc}]}
			/varcalc dist3D ${Math.Distance[${Me.Y},${Me.X},${Me.Z}:${Y_Loc},${X_Loc},${Z_Loc}]}
			|/echo dist now: 2D:${dist2D} 3D:${dist3D}
			/if (${MoveTo.Moving}) {
				/if (${dist3D} > ${distCheck} && ${retryTimer}) {
					/if (${dist2D} > ${distCheck} && ${retryTimer}) {
						|/echo not there yet, returning to loop
						/goto :movingTo_Loop
					} else {
						|/echo dist3D is still far, but 2d is close, I am above/below
						/squelch  /stick off
						/squelch  /moveto off
						/goto :zwarp
					}
				} else {
					/squelch /stick off
					/if (${Debug} && !${spamTimer_MTFAIL}) {
						|/echo I have failed to move to the intended target.
						/call createTimer spamTimer_MTFAIL 30
					}
				}
			} else {
				|/echo not moving ${dist3D} > ${Math.Calc[${distCheck} + ${Math.Calc[${distCheck} / 4].Int}].Int}
				/if (${dist3D} > ${Math.Calc[${distCheck} + ${Math.Calc[${distCheck} / 4].Int}].Int}) {
					|/echo checking retry timer
					/if (${retryTimer}) {
						|/echo (${dist2D} <= ${distCheck}) && ${retryTimer})
						/if ((${dist2D} <= ${distCheck}) && ${retryTimer}) {
							|/echo dist3D is still far, but 2d is close, I am above/below
							/squelch  /stick off
							/squelch  /moveto off
							/goto :zwarp
						} else {
							|/echo going to retry_MoveToLoc
							/goto :retry_MoveToLoc
						}
					} else {
						/if (${Debug} && !${spamTimer_MTFAIL}) {
							/echo I have failed to move to the intended target.
							/call createTimer spamTimer_MTFAIL 30
						}
					}
				} else {
					/squelch /stick off
					/if (${Debug}) /echo I have arrived at my intended destination.
					/delay 30 !${Me.Moving}
				}
			}
		}
	}
:zwarp	
	/squelch  /stick off
	/squelch  /moveto off
	|-- determine the direction (pos, neg to zwarp) Note this is verbose right now for debugging

	/declare tz float local ${Z_Loc}
	/declare mz float local ${Me.Z}
	/declare amt float local 
	/if (${tz}> ${mz}) {
		/varcalc amt ${dist3D}
	} else {
		/varcalc amt ${dist3D}*-1
	}
	/zwarp ${amt}
	
	/if (${Debug}) /echo I have arrived at my intended destination.
	/delay 30 !${Me.Moving}
	
:return
/if (${Debug}) /echo <== MoveTo -|
/return


|----------------------------------------------------------------------------|
|- Triangulates distance between to points using the Pythagoras theorem.	-|
|----------------------------------------------------------------------------|
|- @param loc#1: Location 'A'												-|
|- @param loc#2: Location 'B'												-|
|----------------------------------------------------------------------------|
SUB Triangulate_Distance(string loc#1, string loc#2)
/return ${Math.Sqrt[${Math.Calc[${loc#2.Arg[2,,]}-${loc#1.Arg[2,,]}]}*${Math.Calc[${loc#2.Arg[2,,]}-${loc#1.Arg[2,,]}]}+${Math.Calc[${loc#2.Arg[1,,]}-${loc#1.Arg[1,,]}]}*${Math.Calc[${loc#2.Arg[1,,]}-${loc#1.Arg[1,,]}]}]}

|-- Determine the distance X,Y,Z instead of a single planar X,Y
|-- Formula is AB = SQRT((x2-x1)^2+(y2-y1)^2+(z2-z1)^2)
SUB Triangulate_Distance_3D(string loc#1, string loc#2)
	

/return
SUB argueString(WhatToFind, givenData)
/if (${Debug}) /echo |- argueString ==>

	/declare i int local 1
	/declare foundData string local NULL
	
	:nextArg
	/if (${givenData.Find[/]}) {
		
		/if (${givenData.Arg[${i},/].Find[${WhatToFind}]}) {
		
			/varset foundData ${givenData.Arg[${i},/].Arg[2,|]}
			/if (${Debug}) /echo |- argueString -| Found [${WhatToFind}] - [${foundData}]
		
		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i},/].Length}) /goto :nextArg
		}
	} else {
		/if (${givenData.Arg[${i}, ].Find[${WhatToFind}]}) {
		
			/varset foundData ${givenData.Arg[${i}, ].Arg[2,|]}
			/if (${Debug}) /echo |- argueString -| Found [${WhatToFind}] - [${foundData}]
		
		} else {
			/varcalc i ${i} + 1
			/if (${givenData.Arg[${i}, ].Length}) /goto :nextArg
		}
	}

/if (${Debug}) /echo <== argueString -|
/return ${foundData}

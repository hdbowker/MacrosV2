|**
----------------------------
BUFFING HANDLERS updated version 01.14.22 
----------------------------
This include contains logic for buffing raid/groups

Supported by all classes based on INI settings
CLR/DRU/SHM/PAL/RNG are the buff classes I  use on raid

Will use the section [OOCBuffs] && [TargetBuffs] from ini.
Note if there is no section in INI it will quit out
______________________________________________
REVISION HISTORY
    10.24.16    xiris 	INITIAL REVISION
	11.06.16	xiris	ADDED CHAR BUFFS
	11.17.21	xiris	UPDATED the EVENTS to handle the auto puller checking buffs and asking for them
	12.28.21	xiris	Added Buff Groupings so clients can request "Symbol" and will get whatever the highest symbol the cleric has. Etc.
	01.14.22	xiris	Continued work on buffing logic
	02.15.22	xiris	EQTitan returns nonsense for raid.group stuff now, so we have to make it so we pass in the group leaders names (ugh)
						Added Array named RaidGroupLeaders to store this manually (laaame)
	03.01.22	xiris	Added Exclusions for Buffing use command /only|EXCLUSIONS eg: doRaidBuffs /only|ENC					
**|


#EVENT DORAIDBUFFS "<#1#> doRaidBuffs #2#"
#EVENT DORAIDBUFFS "[#1#(msg)] doRaidBuffs #2#"
#EVENT DORAIDBUFFS "#1# tells you, 'doRaidBuffs #2#'"
#EVENT DORAIDBUFFS "#1# tells the group, 'doRaidBuffs #2#'"
#EVENT DORAIDBUFFS "#1# you tell your party, 'doRaidBuffs #2#'"

#EVENT DOGROUPBUFFS "<#1#> doGroupBuffs #2#"
#EVENT DOGROUPBUFFS "[#1#(msg)] doGroupBuffs #2#"
#EVENT DOGROUPBUFFS "#1# tells you, 'doGroupBuffs #2#'"
#EVENT DOGROUPBUFFS "#1# tells the group, 'doGroupBuffs #2#'"
#EVENT DOGROUPBUFFS "#1# you tell your party, 'doGroupBuffs #2#'"

#EVENT DOSINGLETARGETBUFFS "<#1#> doSingleTargetBuff #2#"
#EVENT DOSINGLETARGETBUFFS "[#1#(msg)] doSingleTargetBuff #2#"
#EVENT DOSINGLETARGETBUFFS "#1# tells you, 'doSingleTargetBuff #2#'"
#EVENT DOSINGLETARGETBUFFS "#1# tells the group, 'doSingleTargetBuff #2#'"
#EVENT DOSINGLETARGETBUFFS "#1# you tell your party, 'doSingleTargetBuff #2#'"

#EVENT DOCHARBUFFS 			"[#1#] doCharBuffs #2#"
#EVENT DOCHARBUFFS 			"[#1#(msg)] doCharBuffs #2#"
#EVENT DOCHARBUFFS 			"<#1#> doCharBuffs #2#"
#EVENT DOCHARBUFFS 			"#1# tells you, 'doCharBuffs #2#'"
#EVENT DOCHARBUFFS 			"#1# tells the group, 'doCharBuffs #2#'"
#EVENT DOCHARBUFFS 			"#1# tells the party, 'doCharBuffs #2#'"

#EVENT RemoveBuff 			"[#1#] Remove buff #2#"
#EVENT RemoveBuff 			"[#1#(msg)] Remove buff #2#"
#EVENT RemoveBuff 			"<#1#> Remove buff #2#"
#EVENT RemoveBuff 			"#1# tells you, 'Remove buff #2#'"
#EVENT RemoveBuff 			"#1# tells the group, 'Remove buff #2#'"
#EVENT RemoveBuff 			"#1# you tell your party, 'Remove buff #2#'"


Sub InitEvents_Buffing
	/call RegisterBackgroundEvents Events_Buffing 5
/return

Sub Events_Buffing
	/doevents DORAIDBUFFS
	/doevents DOGROUPBUFFS
	/doevents DOSINGLETARGETBUFFS
	/doevents DOCHARBUFFS
	/doevents RemoveBuff
/return


Sub EVENT_DORAIDBUFFS(line, ChatSender, eventParams)
	/echo doing raid buffs.
	/if (!${bln_doRaidBuff} && !${bln_doSingleTargetBuff}) /return
	/call verifyEvent "${ChatSender}" "${line}"
	/echo ${Macro.Return}
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	     |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		/echo doing buff
		/memspellset buff
		/delay 3s
		/echo Doing Raid Buffs on ${eventParams}
		/delay 10
		/if (${bln_doRaidBuff}) 		/call doRaidBuffs
		/if (${bln_doSingleTargetBuff}) /call doSingleTargetBuffOnRaid
		/bc ${Me.Class.Name} RAID BUFF [${eventParams}] COMPLETE
		/delay 10
		/memspellset default
	} 
	/doevents flush DORAIDBUFFS
/return

Sub EVENT_DOCHARBUFFS(line, ChatSender, eventParams)
	/if (!${bln_doRaidBuff}) /return
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	     |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		/memspellset buff
		/delay 3s
		/echo Doing Group Buffs on ${eventParams}
		/delay 10
		/call doGroupBuffOnTarget ${eventParams}
		/call doSingleTargetBuffOnChar ${eventParams}
		/bc ${Me.Class.Name} CHARBUFF [${eventParams}] COMPLETE
		/delay 10
		/memspellset default
	}
	/doevents flush DOCHARBUFFS
/return


Sub EVENT_DOGROUPBUFFS
	/if (!${bln_doGroupBuff}) /return
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	     |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		/memspellset buff
		/echo Doing Group Buffs [OOCBUFF]
		/call doGroupBuffOnTarget ${Me.Name}
		/bc ${Me.Class.Name} GROUP BUFF COMPLETE
		/delay 10
		/memspellset default
	}
	/doevents flush DOGROUPBUFFS
/return

Sub EVENT_DOSINGLETARGETBUFFS(line, ChatSender, eventParams)
	/if (!${bln_doSingleTargetBuff}) /return
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	     |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		/memspellset buff
		/if (${Raid.Members}) {
			/call doSingleTargetBuffOnRaid
		} else {
			/call doSingleTargetBuffOnChar ${eventParams}
		}
		/memspellset default
	}
	/doevents flush DOSINGLETARGETBUFFS
/return

Sub EVENT_DOSPECIFICBUFFONCHAR(string _text, string _sender, string _buffer, string _spell);
	|/echo EVENT_DOSPECIFICBUFFONCHAR 
	|/echo _text: ${_text}
	|/echo _sender: ${_sender}
	|/echo _buffer ${_buffer}
	|/echo _spell: ${_spell}
	|/echo ${_buffer} Doing a Specific Buff (${_spell} -> ${_sender}) gb?:${bln_doGroupBuff} nameMatch?:${Me.CleanName.NotEqual[${_buffer}]}
	/if (!${bln_doGroupBuff}) /return
	/if (${Me.CleanName.NotEqual[${_buffer}]}) /return
	
	/memspellset buff
	/call doSpecificBuffOnChar ${_sender} "${_spell}"
	/memspellset default
	/doevents flush DOSPECIFICBUFFONCHAR
/return


|**
 ----------------------------
 Raid Buffs
 ----------------------------
 Will cast the combat buffSpell
 Will run through each raid group and cast the group buff(s) on the group leader
 Note: since ROF2 EMU on Titan 2 is broken vis-a-vis raid stuff we had to change this to a preset array.
 ----------------------------
**|
Sub doRaidBuffs 
	/if (!${bln_doRaidBuff}) /return
	/if (${Raid.Members} <= 6) /return
	/echo Doing Raid Buffs
	/declare i int local 1
	
	/for i 1 to 9
		/echo Targetting group leader (${i}): ${RaidGroupLeaders[${i}]}
		/if (${RaidGroupLeaders[${i}].NotEqual[NULL]}) {
			/call doGroupBuffOnTarget ${RaidGroupLeaders[${i}]}
		}
	/next i
	/call TrueTarget ${int_targetID}
/return


|**
 ----------------------------
 Single Target Combat Buffs
 ----------------------------
 Will cast the combat buffSpell
 as defined by the ini file. Will only cast on the heal target (think tank)
 Note: this is only really for clerics to keep DI on, or Vie
 ----------------------------
**|
Sub cmbtBuffSingle
	/echo Doing Single Target Combat Buffs
		|-- loop through group members that get combat buff
	/declare gmember 	  int 		local
	/declare i    		  int		local
	/declare buffDuration int		local
	/declare buffSpell    string	local
	/declare buffGem	  string	local
	/call healGRP
	
/return

|**
 ----------------------------
 Group Combat Buffs
 ----------------------------
 Will loop through the group and cast the combat buffSpell
 as defined by the ini file. Will only cast on melee (no pets, no casters)
 ----------------------------
**|
Sub cmbtBuffGRP
	/if (!${bln_engaged})     /return
	/if (!${bln_cmbtBuffUse}) /return
	
	|-- loop through group members that get combat buff
	/declare i    		  int		local
	/declare g			  int		local
	/declare gName 	  	  string	local
	/declare buffSpell    string	local
	/declare buffGem	  string	local

	| -- loop through the combat buffs
	/for i 1 to ${int_cmbtBuffTotal}
		| -- check to see if this combat buff is on group members, and if it is missing, slap it on the first appropriate member (if ready)
		/varset		buffSpell	 ${CMBTBUFF_${i}}
		/varset		buffGem		 ${CMBTBUFF_${i}_Gem}
		/call chkSpellMemorized "${buffSpell}" FALSE ${buffGem}

		/for g 0 to ${Group}
			|-- check group health
			/if (${Me.Class.ShortName.Equal[SHM]}) /call healGRP

			|-- Check to see if member has the buff
			/varset gName ${Group.Member[${g}]}
			/call check_validBots melee ${gName}

			/if (${Macro.Return}) {
				/if (${NetBots[${gName}].Buff[${buffSpell}].ID} ) {
					|-- Check to see what the duration is (in ticks)
					/if (${NetBots[${gName}].Buff[${buffSpell}].Duration} < 2) {
						/goto :castCMBTBuff
					} else {
						/goto :nextGroupMember
					}
				} else {
					/goto :castCMBTBuff
				}
			} else {
				/goto :nextGroupMember
			}
			:castCMBTBuff
			/cecho \ayCasting ${buffSpell} on ${gName}
			/call TrueTarget ${NetBots[${gName}].ID}
			/call MQ2Cast "${buffSpell}" ${buffGem}
			/if (${Cast.Result.Equal[CAST_SUCCESS]}) {
				/cecho \ag ${buffSpell} ${Cast.Result} : DELAYING for \ay${NetBots[${gName}].Buff[${buffSpell}].Duration}s \agrefresh	
			}
		:nextGroupMember
		/next g
	:nextCMBT
	/next i
	/call RunRaidEvents
	/call TrueTarget ${int_targetID}
/return


|**
 ----------------------------
 OOC Buffs
 ----------------------------
 Will loop through the group and cast the out of combat buffSpell
 as defined by the ini file. Will only cast once on entire group
 **PH FROM OLD CODE, WILL REWRITE**
 ----------------------------
**|
Sub doGroupBuffOnTarget(string tgt)

	/declare i    		  		int		local
	/declare buffDuration 		int		local
	/declare buffSpell    		string	local
	/declare buffGem	  		string	local
	/declare spellReadyCount	int 	local 0
	/target pc ${tgt}
	/delay 1s
	/for i 1 to ${int_oocBuffTotal}
		/varset 	buffSpell    ${OOCBUFF_${i}}
		/varcalc	buffDuration ${Spell[${OOCBUFF_${i}}].Duration} * 6
		/varset		buffGem		 ${OOCBUFF_${i}_Gem}

		:spellReadyWait
		/varcalc spellReadyCount ${spellReadyCount}+1
		/if (${spellReadyCount} > 50) {
			/echo waited for 5 minutes, it aint happening.
			/varset int_oocBuffTotal 0
			/return
		}
		/if (!${Me.SpellReady[${buffSpell}]} || ${Me.CurrentMana} < ${Spell[${buffSpell}].Mana}) {
			/echo Waiting for ${buffSpell} a Tick (6s)
			/if (${Me.Class.ShortName.Equal[SHM]}) /call chkManaConvert
			/if (${Me.Standing}) /sit
			/delay 6s
			/call RunAllEvents
			/if (${bln_engaged}) /return
			/goto :spellReadyWait
		}

		/if (!${Me.Standing}) /stand on
		/if (${Me.SpellReady[${buffSpell}]} && (${Target.Distance}<=200)) {
			/echo Cast "${buffSpell}" on TARGET: ${tgt}
			/call MQ2Cast "${buffSpell}" ${buffGem} 10s
			/delay 10
			/if (${Cast.Result.Equal[CAST_OUTOFMANA]})  	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_CANNOTSEE]})  	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_OUTOFRANGE]}) 	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_INTERRUPTED]}) 	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_NOTREADY]}) 		/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_SUCCESS]}) {
				/cecho \ag ${buffSpell} ${Cast.Result}
			}
		}
	:NextGB
	/next i
	/call TrueTarget ${int_targetID}
/return		




Sub doSingleTargetBuffOnRaid

	/if (!${bln_doSingleTargetBuff}) /return
	/if (${int_stBuffTotal}==0)		 /return
	
	/echo Doing Single Target Buffs [TargetBuffs] on RAID

	/declare arrBuffTargets[${int_strBuffTotal}]
	/declare	i	int 	local
	/declare    b   int     local
	/declare buffSpell    	string	local
	/declare buffTargets	string	local
	/declare buffGem		string  local
	/declare buffUse		bool	local	TRUE
	/declare charName       string  local
	/declare charClass		string  local
	
		/for b 1 to ${int_stBuffTotal} 
			:startSTBuff
			/varset buffSpell	${STBUFF_${b}}
			/varset	buffTargets	${STBUFF_${b}_Targets}
			/varset buffGem		${STBUFF_${b}_Gem}
			/varset buffUse		${STBUFF_${b}_Use}

			/cecho \ay${buffSpell} ${buffTargets} ${buffGem} ${buffUse}

			/if (!${buffUse}) {
				/if (${b} < ${int_stBuffTotal})  /goto :next_b0
				/if (${b} >= ${int_stBuffTotal}) /return
				/goto :startSTBuff
			}

			/call chkSpellMemorized "${buffSpell}" TRUE ${buffGem}
			
			/for i 1 to ${Raid.Members} 
				/varset charName ${Raid.Member[${i}]}
				/varset charClass ${NetBots[${charName}].Class.ShortName}
				/echo ${charName} ${charClass} : ${buffTargets.Find[${charClass}]}
				/if (${buffTargets.Find[*]} || ${buffTargets.Find[${charClass}]}) {
					/cecho \ag${Raid.Member[${i}]} is a valid Single Buff Target
					/call castSingleTargetBuff ${Raid.Member[${i}]} "${buffSpell}" ${buffGem}
				}
				
			
			/next i
			
		:next_b0
		/next b
	
	/call TrueTarget ${int_targetID}
/return

Sub doSpecificBuffOnChar(string _char, string _spell) 
	/echo doingSpecificBuffOnChar ${_char} ${_spell}
	/delay 10
	/target ${_char}
	/delay 10
	:spellReadyWait
	/if (${Me.SpellReady[${_spell}]} && (${Target.Distance}<=200)) {
		/call MQ2Cast "${_spell}" gem1 10s
	} else {
		/goto :spellReadyWait
	}
	/call TrueTarget ${int_targetID}
/return

Sub doSingleTargetBuffOnChar(string char)

	/if (!${bln_doSingleTargetBuff}) /return
	/if (${int_stBuffTotal}==0)		 /return
	
	/declare	i	int 	local
	/declare    b   int     local
	/declare buffSpell    	string	local
	/declare buffTargets	string	local
	/declare buffGem		string  local
	/declare buffUse		bool	local	TRUE

	
	/for b 1 to ${int_stBuffTotal} 
		:startSTBuf
			/varset buffSpell	${STBUFF_${b}}
			/varset	buffTargets	${STBUFF_${b}_Targets}
			/varset buffGem		${STBUFF_${b}_Gem}
			/varset buffUse		${STBUFF_${b}_Use}
			
			/if (!${buffUse}) {
				/if (${b} < ${int_stBuffTotal})  /goto :next_b1
				/if (${b} >= ${int_stBuffTotal}) /return
				/goto :startSTBuff
			}

			/if (${buffTargets.Find[*]} || (${buffTargets.Find[${NearestSpawn[pc ${char}].Class.ShortName}]} )) {
					/echo ALL CLASSES VALID
					/call castSingleTargetBuff ${char} "${buffSpell}" ${buffGem}
			}
				
			
		:next_b1
		/next b
	/call TrueTarget ${int_targetID}
/return

|-- TODO: make so the errors will cause a counter to be reached (hah! I did this 10 years later! 03.31.22)
Sub castSingleTargetBuff(string tgt, string buffSpell, string buffGem)
	/declare spellReadyCount int local 0
	/target pc ${tgt}
	
	:spellReadyWait2
		/varcalc spellReadyCount ${spellReadyCount}+1
		/if (${spellReadyCount} > 2) {
			/echo waited for 30s, it aint happening.
			/varset int_oocBuffTotal 0
			/return
		}
		/if (!${Me.SpellReady[${buffSpell}]} || ${Me.CurrentMana} < ${Spell[${buffSpell}].Mana}) {
			/echo Waiting for ${buffSpell} a Tick (6s)
			/if (${Me.Class.ShortName.Equal[SHM]}) /call chkManaConvert
			/if (${Me.Standing}) /sit
			/delay 6s
			/call RunAllEvents
			/if (${bln_engaged}) /return
			/goto :spellReadyWait2
		}

		/if (!${Me.Standing}) /stand on
		/if (${Me.SpellReady[${buffSpell}]} && (${Target.Distance}<=200)) {
			/echo Cast "${buffSpell}" on TARGET: ${tgt}
			/call MQ2Cast "${buffSpell}" ${buffGem} 10s
			/delay 10
			/if (${Cast.Result.Equal[CAST_OUTOFMANA]})  	/goto :spellReadyWait2
			/if (${Cast.Result.Equal[CAST_CANNOTSEE]})  	/goto :spellReadyWait2
			/if (${Cast.Result.Equal[CAST_OUTOFRANGE]}) 	/goto :spellReadyWait2
			/if (${Cast.Result.Equal[CAST_INTERRUPTED]}) 	/goto :spellReadyWait2
			/if (${Cast.Result.Equal[CAST_NOTREADY]}) 		/goto :spellReadyWait2
			/if (${Cast.Result.Equal[CAST_SUCCESS]}) {
				/cecho \ag ${buffSpell} ${Cast.Result}
			}
		}
		
	/delay 10
	/call TrueTarget ${int_targetID}
/return

|-- Determine if I have a buff by type, and if not
Sub chkBuffByType(string _key) 

	/echo chkBuffByType(${_key})

	/declare hasBuffType bool local FALSE
	/declare buffToCheck string local
	/declare li listiterator
	

	/if (${_key.Equal[aego]}) {
		/vardata li l_aegolism.First
		/echo checking aego
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[symbol]}) {
		/echo checking symbol
		/vardata li l_symbol.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[acbuff]}) {
		/echo checking acbuff
		/vardata li l_clrAC.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}

	} else /if (${_key.Equal[nine]}) {
		/echo checking nine
		/vardata li l_nine.First

		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[encHaste]}) {
		/echo checking encHaste
		/vardata li l_encHaste.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[shmHaste]}) {
		/echo checking shmHaste
		/vardata li l_shmHaste.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[focus]}) {
		/echo checking focus
		/vardata li l_focus.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	
	} else /if (${_key.Equal[brells]}) {
		/echo checking brells
		/vardata li l_brells.First
		/while (!${li.IsEnd}) {
			/echo ${li.Value}
			/varset buffToCheck ${li.Value}
			/if (${li.Advance}) {
				/echo 'Iterator advanced to next element.'
			}
		}
	}
/return ${hasBuffType}



|-- check OOC Buffs for myself
Sub chkOOCBuffs

	/declare i    		  int		local
	/declare buffDuration int		local
	/declare buffSpell    string	local
	/declare buffGem	  string	local

	/for i 1 to ${int_oocBuffTotal}
		/varset 	buffSpell    ${OOCBUFF_${i}}
		|/echo ${buffSpell}
		/if (${Me.Buff["${buffSpell}"].ID}) {
			|/echo already have buff, doing next
			/goto :next
		}
		/varcalc	buffDuration ${Spell[${OOCBUFF_${i}}].Duration} * 6
		/varset		buffGem		 ${OOCBUFF_${i}_Gem}

		:spellReadyWait
		/memspellset buff
		/delay 3s
		/if (!${Me.SpellReady["${buffSpell}"]} || ${Me.CurrentMana} < ${Spell[${buffSpell}].Mana}) {
			/echo Waiting for ${buffSpell} for SELF buff
			/if (${Me.Standing}) /sit
			/call RunAllEvents
			/if (${bln_engaged}) /return
			/goto :spellReadyWait
		}

		/if (!${Me.Standing}) /stand on
		/keypress f1
		/if (${Me.SpellReady[${buffSpell}]}) {
			/echo Cast "${buffSpell}" on MYSELF
			/call MQ2Cast "${buffSpell}" ${buffGem} 10s
			/delay 10
			/if (${Cast.Result.Equal[CAST_OUTOFMANA]})  	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_CANNOTSEE]})  	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_OUTOFRANGE]}) 	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_INTERRUPTED]}) 	/goto :spellReadyWait
			/if (${Cast.Result.Equal[CAST_NOTREADY]}) 		/goto :spellReadyWait
		}
		
	:next
	/next i
	/memspellset default
	/call TrueTarget ${int_targetID}
/return



|----------------------------------------------------|
|- Attempts to remove specified buffs on NetBots.	-|
|----------------------------------------------------|
|- Use, /bc Remove [Name], or /bc Remove 'Name'.	-|
|----------------------------------------------------|

Sub EVENT_RemoveBuff(line, ChatSender, eventParams)
	/declare badBuff string local ${eventParams.Arg[1,-]}
	/call RemoveBuff ${badBuff}

/return

Sub RemoveBuff(_badBuff)
	/declare i int local

	/for i 0 to 24
		|/echo RemoveBuff ${i}
		|/echo 1: ${Bool[${Window[BuffWindow].Child[BW_Buff${i}_Button].Tooltip}]}
		|/echo 2: ${Window[BuffWindow].Child[BW_Buff${i}_Button].Tooltip.Find[${_badBuff}]}
		/if (${Bool[${Window[BuffWindow].Child[BW_Buff${i}_Button].Tooltip}]} && ${Window[BuffWindow].Child[BW_Buff${i}_Button].Tooltip.Find[${_badBuff}]}) {
			/varset _badBuff ${Window[BuffWindow].Child[BW_Buff${i}_Button].Tooltip.Arg[1,(]}
			/varset _badBuff ${_badBuff.Left[${Math.Calc[${_badBuff.Length}-1]}]}
			/if (${Bool[${_badBuff}]}) /nomodkey /notify BuffWindow Buff${Math.Calc[${Me.Buff[${_badBuff}].ID}-1].Int} leftmouseup
		}
		
	/next i
/return




Sub xbot_initialize_buffing
	/cecho \aoxbot_initialize_buffing
	/call InitEvents_Buffing
	
	|-- Manually set an array with the standard group leaders(ugh)
	| 	Xiris,Xiria,Xirian,Xeixa,Sadelia,Xaeliea,Xaexa,Xaia,Xepatia
	| 	in the future maybe have an event that can set this
	/declare RaidGroupLeaders[9] string outer NULL 
	/varset	RaidGroupLeaders[1] Xiris
	/varset	RaidGroupLeaders[2] Xiria
	/varset RaidGroupLeaders[3] Theist
	/varset RaidGroupLeaders[4] Xeixa
	/varset RaidGroupLeaders[5] Sadelia
	/varset RaidGroupLeaders[6] Xaeliea
	/varset RaidGroupLeaders[7] Xaexa
	/varset RaidGroupLeaders[8] Xaia
	/varset RaidGroupLeaders[9] Xepatia

	| |--Buff Chains - Characters can check on this list to see if they have a buff archetype
	| /declare l_aegolism	list outer
	| /declare str_aegolism_request	string	outer 	aego
	| /if (${l_aegolism.Append[Blessing of Aegolism,Hand of Virtue,Hand of Conviction]}) {
	| 	|/echo 'l_aegolism Append succeeded. Count: ${l_aegolism.Count}.'
	| } else {
	| 	/echo 'l_aegolism List insertion failed.'
	| }

	| /declare l_symbol list	outer
	| /declare str_symbol_request		string	outer	symbol
	| /if (${l_symbol.Append[Marzin's Mark,Kazad's Mark,Balikor's Mark]}) {
	| 	|/echo 'l_symbol Append succeeded. Count: ${l_symbol.Count}.'
	| } else {
	| 	/echo 'l_symbol List insertion failed.'
	| }

	| /declare l_clrAC list outer
	| /declare str_ac_request			string	outer	acbuff
	| /if (${l_clrAC.Append[Bulwark of Faith,Ward of Gallantry,Ward of Valiance]}) {
	| 	|/echo 'l_clrAC Append succeeded. Count: ${l_clrAC.Count}.'
	| } else {
	| 	/echo 'l_clrAC List insertion failed.'
	| }

	| /declare l_nine	list outer	
	| /declare str_nine_request		string	outer	nine
	| /if (${l_nine.Append[Blessing of the Nine,Blessing of Steeloak]}) {
	| 	|/echo 'l_nine Append succeeded. Count: ${l_nine.Count}.'
	| } else {
	| 	/echo 'l_nine List insertion failed.'
	| }
	
	| /declare l_encHaste	list outer
	| /declare str_encHaste_request	string	outer	encHaste
	| /if (${l_encHaste.Append[Speed of the Brood,Vallon's Quickening,Hastening of Salik]}) {
	| 	|/echo 'l_encHaste Append succeeded. Count: ${l_encHaste.Count}.'
	| } else {
	| 	/echo 'l_encHaste List insertion failed.'
	| }
	
	| /declare l_shmHaste list outer
	| /declare str_shmHaste_request	string	outer	shmHaste
	| /if (${l_shmHaste.Append[Talisman of Celerity]}) {
	| 	|/echo 'l_shmHaste Append succeeded. Count: ${l_shmHaste.Count}.'
	| } else {
	| 	/echo 'l_shmHaste List insertion failed.'
	| }

	
	| /declare l_focus list outer
	| /declare str_focus_request		string	outer	focus
	| /if (${l_focus.Append[Focus of the Seventh, Talisman of Wunshi]}) {
	| 	|/echo 'l_focus Append succeeded. Count: ${l_focus.Count}.'
	| } else {
	| 	/echo 'l_focus List insertion failed.'
	| }

	
	| /declare l_brells list outer
	| /declare str_brells_request		string	outer	brells
	| /if (${l_brells.Append[Brell's Mountainous Barrier,Brell's Stalwart Shield,Brell's Brawny Bulwark]}) {
	| 	|/echo 'l_brells Append succeeded. Count: ${l_brells.Count}.'
	| } else {
	| 	/echo 'l_brells List insertion failed.'
	| }

	
	|/call chkBuffByType nine
	|/echo checking if we have nine ${Macro.Return}

	|--OOC Buff(s)
	/declare bln_doRaidBuff				bool	outer	${Ini[botINI,OOCBuffs,bln_doRaidBuff,FALSE]}
	/declare bln_doGroupBuff			bool	outer	${Ini[botINI,OOCBuffs,bln_doGroupBuff,FALSE]}
	/declare bln_doAutoBuff				bool	outer	${Ini[botINI,OOCBuffs,bln_doGroupBuff,FALSE]}
	/declare int_oocBuffTotal			int		outer	${Ini[botINI,OOCBuffs,int_oocBuffTotal,0]}
	
	/if (${int_oocBuffTotal} > 0) {
		/declare ooct						int		local
		/for ooct 1 to ${int_oocBuffTotal} 
			/declare OOCBUFF_${ooct} 				string	outer	${Ini[botINI,OOCBuffs,OOCBUFF_${ooct}]}
			/declare OOCBUFF_${ooct}_Use 			bool	outer	${Ini[botINI,OOCBuffs,OOCBUFF_${ooct}_Use]}
			/declare OOCBUFF_${ooct}_Gem 			string	outer	${Ini[botINI,OOCBuffs,OOCBUFF_${ooct}_Gem]}
			/declare OOCBUFF_${ooct}_Active			bool	outer 	FALSE
			/declare OOCBUFF_${ooct}_Timer	timer   outer   0
		/next ooct
	}
	
	|--Single Target Buff(s)
	|--eg: Divine Intervention, Mage Huge DS, Ranger AC Buff, Cleric AC buff (if no group version)
	/declare bln_doSingleTargetBuff		bool	outer	${Ini[botINI,TargetBuffs,bln_doSingleTargetBuff,FALSE]}
	/declare int_stBuffTotal			int		outer	${Ini[botINI,TargetBuffs,int_stBuffTotal,0]}
	
	/echo ${int_stBuffTotal}
	/if (${int_stBuffTotal} > 0) {
		/declare stbuff						int		local
		/for stbuff 1 to ${int_stBuffTotal} 
			/declare STBUFF_${stbuff} 				string	outer	${Ini[botINI,TargetBuffs,STBUFF_${stbuff}]}
			/declare STBUFF_${stbuff}_Use 			bool	outer	${Ini[botINI,TargetBuffs,STBUFF_${stbuff}_Use]}
			/declare STBUFF_${stbuff}_Gem 			string	outer	${Ini[botINI,TargetBuffs,STBUFF_${stbuff}_Gem]}
			/declare STBUFF_${stbuff}_Targets		string	outer	${Ini[botINI,TargetBuffs,STBUFF_${stbuff}_Targets]}
			/echo ${STBUFF_${stbuff}_Targets}
			/declare STBUFF_${stbuff}_Active		bool	outer 	FALSE
			/declare STBUFF_${stbuff}_Timer	timer   outer   0
		/next stbuff
	}
	
	
	|--Combat Buff(s)
	|--eg: Spirit of the Jaguar
	/declare bln_cmbtBuffUse			bool	outer	${Ini[botINI,CombatBuffs,bln_cmbtBuffUse,TRUE]}
	/declare int_cmbtBuffTotal			int		outer	${Ini[botINI,CombatBuffs,int_cmbtBuffTotal,0]}
	/declare cmbt						int		local
	/declare cmbtG						int		local
	
	/for cmbt 1 to ${int_cmbtBuffTotal} 
		/declare CMBTBUFF_${cmbt} 				string	outer	${Ini[botINI,CombatBuffs,CMBTBUFF_${cmbt}]}
		/declare CMBTBUFF_${cmbt}_Use 			bool	outer	${Ini[botINI,CombatBuffs,CMBTBUFF_${cmbt}_Use]}
		/declare CMBTBUFF_${cmbt}_Gem 			string	outer	${Ini[botINI,CombatBuffs,CMBTBUFF_${cmbt}_Gem]}
		/declare CMBTBUFF_${cmbt}_Active		bool	outer 	FALSE
		
		/for cmbtG 0 to ${Group}
			/declare CMBTBUFF_${cmbt}_${cmbtG}_Timer	timer   outer   0
		/next cmbtG
	
	/next cmbt
	
	
/return









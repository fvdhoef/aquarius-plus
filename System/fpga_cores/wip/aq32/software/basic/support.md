# Aquarius32 BASIC support of QBASIC functionality

## TODO

Editor:

- Undo / Redo
- Replace

General:

- Saving of executable (runtime + bytecode)

Timer:

- TIMER statement
- ON TIMER statement

Flow control:

- ON expr GOSUB
- ON expr GOTO
- DO...LOOP
- EXIT DO/FOR

String:

- MID$ statement

Error handling:

- RESUME
- ERL
- ERR

Graphics:

- SCREEN
- CIRCLE
- LINE
- DRAW
- PAINT

## Control flow

|  ✓  | Name                               | Type      |
| :-: | ---------------------------------- | --------- |
|     | CALL                               | Statement |
|     | DECLARE                            | Statement |
|     | DEF FN                             | Statement |
|     | DO...WHILE\|UNTIL...LOOP           | Statement |
|  ✓  | END                                | Statement |
|     | EXIT                               | Statement |
|  ✓  | FOR...TO...STEP...NEXT             | Statement |
|     | FUNCTION                           | Statement |
|  ✓  | GOSUB                              | Statement |
|  ✓  | GOTO                               | Statement |
|  ✓  | IF...THEN...ELSEIF...ELSE...END IF | Statement |
|     | ON...GOSUB                         | Statement |
|     | ON...GOTO                          | Statement |
|  ✓  | REM                                | Statement |
|  ✓  | RETURN                             | Statement |
|     | RUN                                | Statement |
|     | SELECT CASE                        | Statement |
|     | SHELL                              | Statement |
|     | STOP                               | Statement |
|     | SUB                                | Statement |
|     | SYSTEM                             | Statement |
|     | TRON                               | Statement |
|     | TROFF                              | Statement |
|  ✓  | WHILE...WEND                       | Statement |

## Variables, data and types

|  ✓  | Name        | Type        |
| :-: | ----------- | ----------- |
|     | LBOUND      | Function    |
|     | UBOUND      | Function    |
|     | $DYNAMIC    | Metacommand |
|     | $STATIC     | Metacommand |
|     | CLEAR       | Statement   |
|     | COMMON      | Statement   |
|     | CONST       | Statement   |
|  ✓  | DATA        | Statement   |
|  ✓  | DIM         | Statement   |
|     | ERASE       | Statement   |
|  ✓  | LET         | Statement   |
|     | OPTION BASE | Statement   |
|  ✓  | READ        | Statement   |
|     | REDIM       | Statement   |
|  ✓  | RESTORE     | Statement   |
|     | SHARED      | Statement   |
|     | STATIC      | Statement   |
|  ✓  | SWAP        | Statement   |

### Data types

|  ✓  | Name                                              | Type      |
| :-: | ------------------------------------------------- | --------- |
|  ✓  | CDBL                                              | Function  |
|  ✓  | CINT                                              | Function  |
|  ✓  | CLNG                                              | Function  |
|  ✓  | CSNG                                              | Function  |
|  ✓  | DEFDBL                                            | Statement |
|  ✓  | DEFINT                                            | Statement |
|  ✓  | DEFLNG                                            | Statement |
|  ✓  | DEFSNG                                            | Statement |
|  ✓  | DEFSTR                                            | Statement |
|     | TYPE ... AS INTEGER\|LONG\|SINGLE\|DOUBLE\|STRING | Statement |

## Error handling

|  ✓  | Name     | Type       | Note          |
| :-: | -------- | ---------- | ------------- |
|     | ERL      | Function   |               |
|     | ERR      | Function   |               |
|  ✓  | ERROR    | Statement  |               |
|     | ON ERROR | Statement  |               |
|     | RESUME   | Statement  |               |
|     | _ERDEV_  | _Function_ | _Unsupported_ |
|     | _ERDEV$_ | _Function_ | _Unsupported_ |

## Math

|  ✓  | Name      | Type      |
| :-: | --------- | --------- |
|  ✓  | ABS       | Function  |
|  ✓  | ATN       | Function  |
|  ✓  | COS       | Function  |
|  ✓  | EXP       | Function  |
|  ✓  | FIX       | Function  |
|  ✓  | INT       | Function  |
|  ✓  | LOG       | Function  |
|  ✓  | RND       | Function  |
|  ✓  | SGN       | Function  |
|  ✓  | SIN       | Function  |
|  ✓  | SQR       | Function  |
|  ✓  | TAN       | Function  |
|  ✓  | AND       | Operator  |
|  ✓  | EQV       | Operator  |
|  ✓  | IMP       | Operator  |
|  ✓  | MOD       | Operator  |
|  ✓  | NOT       | Operator  |
|  ✓  | OR        | Operator  |
|  ✓  | XOR       | Operator  |
|  ✓  | RANDOMIZE | Statement |

## String manipulation

|  ✓  | Name    | Type      |
| :-: | ------- | --------- |
|  ✓  | ASC     | Function  |
|  ✓  | CHR$    | Function  |
|  ✓  | HEX$    | Function  |
|  ✓  | INSTR   | Function  |
|  ✓  | LCASE$  | Function  |
|  ✓  | LEFT$   | Function  |
|  ✓  | LEN     | Function  |
|  ✓  | LTRIM$  | Function  |
|  ✓  | MID$    | Function  |
|  ✓  | OCT$    | Function  |
|  ✓  | RIGHT$  | Function  |
|  ✓  | RTRIM$  | Function  |
|  ✓  | SPACE$  | Function  |
|  ✓  | STR$    | Function  |
|  ✓  | STRING$ | Function  |
|  ✓  | UCASE$  | Function  |
|  ✓  | VAL     | Function  |
|     | LSET    | Statement |
|     | MID$    | Statement |
|     | RSET    | Statement |

## Console I/O and graphics

|  ✓  | Name              | Type      |
| :-: | ----------------- | --------- |
|  ✓  | CSRLIN            | Function  |
|  ✓  | INKEY$            | Function  |
|     | PMAP              | Function  |
|     | POINT             | Function  |
|  ✓  | POS               | Function  |
|     | SCREEN            | Function  |
|  ✓  | SPC               | Function  |
|  ✓  | TAB               | Function  |
|     | CIRCLE            | Statement |
|  ✓  | CLS               | Statement |
|  ✓  | COLOR             | Statement |
|     | DRAW              | Statement |
|     | GET               | Statement |
|  ✓  | INPUT             | Statement |
|     | KEY               | Statement |
|     | KEY LIST\|ON\|OFF | Statement |
|     | KEY()             | Statement |
|     | LINE              | Statement |
|  ✓  | LINE INPUT        | Statement |
|  ✓  | LOCATE            | Statement |
|     | ON KEY()          | Statement |
|     | PAINT             | Statement |
|     | PALETTE           | Statement |
|     | PALETTE USING     | Statement |
|     | PCOPY             | Statement |
|     | PRESET            | Statement |
|  ✓  | PRINT             | Statement |
|     | PRINT USING       | Statement |
|     | PSET              | Statement |
|     | PUT               | Statement |
|     | SCREEN            | Statement |
|     | VIEW              | Statement |
|     | VIEW PRINT        | Statement |
|  ✓  | WIDTH             | Statement |
|     | WINDOW            | Statement |
|     | WRITE             | Statement |

## File I/O

|  ✓  | Name                            | Type        | Note                                                                   |
| :-: | ------------------------------- | ----------- | ---------------------------------------------------------------------- |
|  ✓  | CVI                             | Function    |                                                                        |
|  ✓  | CVL                             | Function    |                                                                        |
|  ✓  | CVS                             | Function    |                                                                        |
|  ✓  | CVD                             | Function    |                                                                        |
|  ✓  | MKI$                            | Function    |                                                                        |
|  ✓  | MKL$                            | Function    |                                                                        |
|  ✓  | MKS$                            | Function    |                                                                        |
|  ✓  | MKD$                            | Function    |                                                                        |
|  ✓  | EOF                             | Function    |                                                                        |
|     | FILEATTR                        | Function    |                                                                        |
|  ✓  | INPUT$                          | Function    |                                                                        |
|     | LOC                             | Function    |                                                                        |
|  ✓  | LOF                             | Function    |                                                                        |
|  ✓  | SEEK                            | Function    |                                                                        |
|     | BLOAD                           | Statement   |                                                                        |
|     | BSAVE                           | Statement   |                                                                        |
|     | CHAIN                           | Statement   |                                                                        |
|  ✓  | CHDIR                           | Statement   |                                                                        |
|  ✓  | CLOSE                           | Statement   |                                                                        |
|     | FIELD                           | Statement   |                                                                        |
|     | FILES                           | Statement   |                                                                        |
|     | GET #                           | Statement   |                                                                        |
|  ✓  | INPUT #                         | Statement   |                                                                        |
|  ✓  | KILL                            | Statement   |                                                                        |
|  ✓  | LINE INPUT #                    | Statement   |                                                                        |
|  ✓  | MKDIR                           | Statement   |                                                                        |
|     | NAME                            | Statement   |                                                                        |
|  ✓  | OPEN                            | Statement   |                                                                        |
|  ✓  | PRINT #                         | Statement   |                                                                        |
|     | PUT #                           | Statement   |                                                                        |
|  ✓  | READ # (not in QBASIC, remove?) | Statement   |                                                                        |
|     | RESET                           | Statement   |                                                                        |
|  ✓  | RMDIR                           | Statement   |                                                                        |
|  ✓  | SEEK                            | Statement   |                                                                        |
|  ✓  | WRITE #                         | Statement   |                                                                        |
|     | _LOCK_                          | _Statement_ | _File locking unsupported_                                             |
|     | _UNLOCK_                        | _Statement_ | _File locking unsupported_                                             |
|     | _IOCTL_                         | _Statement_ | _Unsupported by ESP_                                                   |
|     | _IOCTL$_                        | _Function_  | _Unsupported by ESP_                                                   |
|     | _FREEFILE_                      | _Function_  | _Unneccessary as Aq32 BASIC OPEN will return the file number instead._ |
|     | _CVSMBF_                        | _Function_  | _Won't support legacy MBF floating point format_                       |
|     | _CVDMBF_                        | _Function_  | _Won't support legacy MBF floating point format_                       |
|     | _MKSMBF$_                       | _Function_  | _Won't support legacy MBF floating point format_                       |
|     | _MKDMBF$_                       | _Function_  | _Won't support legacy MBF floating point format_                       |

## Time related

|  ✓  | Name                | Type        | Note                                       |
| :-: | ------------------- | ----------- | ------------------------------------------ |
|     | DATE$               | Function    |                                            |
|     | TIME$               | Function    |                                            |
|  ✓  | TIMER               | Function    |                                            |
|     | ON TIMER            | Statement   |                                            |
|     | SLEEP               | Statement   |                                            |
|     | TIMER ON\|OFF\|STOP | Statement   |                                            |
|     | _DATE$_             | _Statement_ | _Aq32 doesn't allow manually setting date_ |
|     | _TIME$_             | _Statement_ | _Aq32 doesn't allow manually setting time_ |

## Sound

|  ✓  | Name               | Type      |
| :-: | ------------------ | --------- |
|     | PLAY               | Function  |
|     | BEEP               | Statement |
|     | ON PLAY            | Statement |
|     | PLAY               | Statement |
|     | PLAY ON\|OFF\|STOP | Statement |
|     | SOUND              | Statement |

## Joystick

|  ✓  | Name                | Type      |
| :-: | ------------------- | --------- |
|     | STICK               | Function  |
|     | STRIG               | Function  |
|     | ON STRIG            | Statement |
|     | STRIG ON\|OFF\|STOP | Statement |

## Low level

|  ✓  | Name                | Type        | Note                            |
| :-: | ------------------- | ----------- | ------------------------------- |
|     | ENVIRON$            | Function    |                                 |
|     | FRE                 | Function    |                                 |
|     | PEEK                | Function    |                                 |
|     | VARPTR              | Function    |                                 |
|     | VARPTR$             | Function    |                                 |
|     | CALL ABSOLUTE       | Statement   |                                 |
|     | ENVIRON             | Statement   |                                 |
|     | POKE                | Statement   |                                 |
|     | _COM ON\|OFF\|STOP_ | _Statement_ | _Aq32 doesn't have serial port_ |
|     | _OPEN COM_          | _Statement_ | _Aq32 doesn't have serial port_ |
|     | _ON COM_            | _Statement_ | _Aq32 doesn't have serial port_ |
|     | _PEN_               | _Function_  | _Aq32 doesn't have light pen_   |
|     | _PEN_               | _Statement_ | _Aq32 doesn't have light pen_   |
|     | _ON PEN_            | _Statement_ | _Aq32 doesn't have light pen_   |
|     | _DEF SEG_           | _Statement_ | _RISC-V doesn't have segments_  |
|     | _VARSEG_            | _Function_  | _RISC-V doesn't have segments_  |
|     | _INP_               | _Function_  | _RISC-V doesn't have ports_     |
|     | _OUT_               | _Statement_ | _RISC-V doesn't have ports_     |
|     | _WAIT_              | _Statement_ | _RISC-V doesn't have ports_     |

## Printer

|  ✓  | Name         | Type      |
| :-: | ------------ | --------- |
|     | LPOS         | Function  |
|     | LPRINT       | Statement |
|     | LPRINT USING | Statement |

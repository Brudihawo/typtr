#ifndef KEYS_H
#define KEYS_H

#define N_CHARS (128 - 32 - 1)
int char_idx(char c);

typedef enum {
  KC_NUL = 0, //< Null Character
  KC_SOH, //< Start of Header
  KC_STX, //< Start of Text
  KC_ETX, //< End of Text
  KC_EOT, //< End of Transmission
  KC_ENQ, //< Enquiry
  KC_ACK, //< Acknowledge
  KC_BEL, //< Bell
  KC_BS,  //< Backspace
  KC_HT,  //< Horizontal Tab
  KC_LF,  //< Line Feed
  KC_VT,  //< Vertical Tab
  KC_FF,  //< Form Feed
  KC_CR,  //< Carriage Return
  KC_SO,  //< Shift In
  KC_SI,  //< Shift Out
  KC_DLE, //< Data Link Escape
  KC_DC1, //< Device Control 1
  KC_DC2, //< Device Control 2
  KC_DC3, //< Device Control 3
  KC_DC4, //< Device Control 4
  KC_NAK, //< Negative Acknowledge
  KC_SYN, //< Synchronize
  KC_ETB, //< End of Transmission Block
  KC_CAN, //< Cancel
  KC_EM,  //< End of Medium
  KC_SUB, //< Substitute
  KC_ESC, //< Escape
  KC_FS,  //< File Separator
  KC_GS,  //< Group Separator
  KC_RS,  //< Record Separator
  KC_US,  //< Unit Separator
  KC_SPC, //< SPACE
  KC_EXC, //< Exclamation point
  KC_DQT, //< Double Quotation mark
  KC_PND, //< Pound
  KC_DOL, //< Dollar
  KC_PCT, //< Percent
  KC_AMP, //< Ampersand / and
  KC_SQT, //< Single Quote
  KC_ORB, //< Open Round Bracket
  KC_CRB, //< Closed Round Bracket
  KC_STR, //< Star
  KC_PLS, //< Plus
  KC_CMA, //< Comma
  KC_MNS, //< Minus
  KC_DOT, //< Dot
  KC_SLS, //< Slash
  KC_0,   //< 0
  KC_1,   //< 1
  KC_2,   //< 2
  KC_3,   //< 3
  KC_4,   //< 4
  KC_5,   //< 5
  KC_6,   //< 6
  KC_7,   //< 7
  KC_8,   //< 8
  KC_9,   //< 9
  KC_COL, //< Colon
  KC_SCL, //< Semicolon
  KC_GT,  //< Greater than
  KC_EQ,  //< Equals
  KC_LT,  //< Less than
  KC_QMK, //< Question mark
  KC_AT,  //< At sign
  KC_UPA, //< Upper Case A
  KC_UPB, //< Upper Case B
  KC_UPC, //< Upper Case C
  KC_UPD, //< Upper Case D
  KC_UPE, //< Upper Case E
  KC_UPF, //< Upper Case F
  KC_UPG, //< Upper Case G
  KC_UPH, //< Upper Case H
  KC_UPI, //< Upper Case I
  KC_UPJ, //< Upper Case J
  KC_UPK, //< Upper Case K
  KC_UPL, //< Upper Case L
  KC_UPM, //< Upper Case M
  KC_UPN, //< Upper Case N
  KC_UPO, //< Upper Case O
  KC_UPP, //< Upper Case P
  KC_UPQ, //< Upper Case Q
  KC_UPR, //< Upper Case R
  KC_UPS, //< Upper Case S
  KC_UPT, //< Upper Case T
  KC_UPU, //< Upper Case U
  KC_UPV, //< Upper Case V
  KC_UPW, //< Upper Case W
  KC_UPX, //< Upper Case X
  KC_UPY, //< Upper Case Y
  KC_UPZ, //< Upper Case Z
  KC_OSB, //< Open Square Bracket
  KC_BSL, //< Backslash
  KC_CSB, //< Closed Square Bracket
  KC_CAR, //< Caret
  KC_USC, //< Underscore
  KC_GRV, //< Grave / backtick
  KC_LOA, //< Lower Case A
  KC_LOB, //< Lower Case B
  KC_LOC, //< Lower Case C
  KC_LOD, //< Lower Case D
  KC_LOE, //< Lower Case E
  KC_LOF, //< Lower Case F
  KC_LOG, //< Lower Case G
  KC_LOH, //< Lower Case H
  KC_LOI, //< Lower Case I
  KC_LOJ, //< Lower Case J
  KC_LOK, //< Lower Case K
  KC_LOL, //< Lower Case L
  KC_LOM, //< Lower Case M
  KC_LON, //< Lower Case N
  KC_LOO, //< Lower Case O
  KC_LOP, //< Lower Case P
  KC_LOQ, //< Lower Case Q
  KC_LOR, //< Lower Case R
  KC_LOS, //< Lower Case S
  KC_LOT, //< Lower Case T
  KC_LOU, //< Lower Case U
  KC_LOV, //< Lower Case V
  KC_LOW, //< Lower Case W
  KC_LOX, //< Lower Case X
  KC_LOY, //< Lower Case Y
  KC_LOZ, //< Lower Case Z
  KC_OCB, //< Open Curly Brace
  KC_PIP, //< Pipe
  KC_CCB, //< Closed Curly Brace
  KC_TLD, //< Tilde
  KC_DEL, //< Delete
} KeyCode;

#endif // KEYS_H

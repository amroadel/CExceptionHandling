#define DW_FIRST_OP(name, value) enum dwarf_location_atom { \
  name = value
#define DW_END_OP };

#define DW_FIRST_CFA(name, value) enum dwarf_call_frame_info { \
  name = value
#define DW_END_CFA };

#define DW_OP(name, value) , name = value
#define DW_OP_DUP(name, value) , name = value

#define DW_CFA(name, value) , name = value
#define DW_CFA_DUP(name, value) , name = value

DW_FIRST_OP (DW_OP_addr, 0x03)
DW_OP (DW_OP_deref, 0x06)
DW_OP (DW_OP_const1u, 0x08)
DW_OP (DW_OP_const1s, 0x09)
DW_OP (DW_OP_const2u, 0x0a)
DW_OP (DW_OP_const2s, 0x0b)
DW_OP (DW_OP_const4u, 0x0c)
DW_OP (DW_OP_const4s, 0x0d)
DW_OP (DW_OP_const8u, 0x0e)
DW_OP (DW_OP_const8s, 0x0f)
DW_OP (DW_OP_constu, 0x10)
DW_OP (DW_OP_consts, 0x11)
DW_OP (DW_OP_dup, 0x12)
DW_OP (DW_OP_drop, 0x13)
DW_OP (DW_OP_over, 0x14)
DW_OP (DW_OP_pick, 0x15)
DW_OP (DW_OP_swap, 0x16)
DW_OP (DW_OP_rot, 0x17)
DW_OP (DW_OP_xderef, 0x18)
DW_OP (DW_OP_abs, 0x19)
DW_OP (DW_OP_and, 0x1a)
DW_OP (DW_OP_div, 0x1b)
DW_OP (DW_OP_minus, 0x1c)
DW_OP (DW_OP_mod, 0x1d)
DW_OP (DW_OP_mul, 0x1e)
DW_OP (DW_OP_neg, 0x1f)
DW_OP (DW_OP_not, 0x20)
DW_OP (DW_OP_or, 0x21)
DW_OP (DW_OP_plus, 0x22)
DW_OP (DW_OP_plus_uconst, 0x23)
DW_OP (DW_OP_shl, 0x24)
DW_OP (DW_OP_shr, 0x25)
DW_OP (DW_OP_shra, 0x26)
DW_OP (DW_OP_xor, 0x27)
DW_OP (DW_OP_bra, 0x28)
DW_OP (DW_OP_eq, 0x29)
DW_OP (DW_OP_ge, 0x2a)
DW_OP (DW_OP_gt, 0x2b)
DW_OP (DW_OP_le, 0x2c)
DW_OP (DW_OP_lt, 0x2d)
DW_OP (DW_OP_ne, 0x2e)
DW_OP (DW_OP_skip, 0x2f)
DW_OP (DW_OP_lit0, 0x30)
DW_OP (DW_OP_lit1, 0x31)
DW_OP (DW_OP_lit2, 0x32)
DW_OP (DW_OP_lit3, 0x33)
DW_OP (DW_OP_lit4, 0x34)
DW_OP (DW_OP_lit5, 0x35)
DW_OP (DW_OP_lit6, 0x36)
DW_OP (DW_OP_lit7, 0x37)
DW_OP (DW_OP_lit8, 0x38)
DW_OP (DW_OP_lit9, 0x39)
DW_OP (DW_OP_lit10, 0x3a)
DW_OP (DW_OP_lit11, 0x3b)
DW_OP (DW_OP_lit12, 0x3c)
DW_OP (DW_OP_lit13, 0x3d)
DW_OP (DW_OP_lit14, 0x3e)
DW_OP (DW_OP_lit15, 0x3f)
DW_OP (DW_OP_lit16, 0x40)
DW_OP (DW_OP_lit17, 0x41)
DW_OP (DW_OP_lit18, 0x42)
DW_OP (DW_OP_lit19, 0x43)
DW_OP (DW_OP_lit20, 0x44)
DW_OP (DW_OP_lit21, 0x45)
DW_OP (DW_OP_lit22, 0x46)
DW_OP (DW_OP_lit23, 0x47)
DW_OP (DW_OP_lit24, 0x48)
DW_OP (DW_OP_lit25, 0x49)
DW_OP (DW_OP_lit26, 0x4a)
DW_OP (DW_OP_lit27, 0x4b)
DW_OP (DW_OP_lit28, 0x4c)
DW_OP (DW_OP_lit29, 0x4d)
DW_OP (DW_OP_lit30, 0x4e)
DW_OP (DW_OP_lit31, 0x4f)
DW_OP (DW_OP_reg0, 0x50)
DW_OP (DW_OP_reg1, 0x51)
DW_OP (DW_OP_reg2, 0x52)
DW_OP (DW_OP_reg3, 0x53)
DW_OP (DW_OP_reg4, 0x54)
DW_OP (DW_OP_reg5, 0x55)
DW_OP (DW_OP_reg6, 0x56)
DW_OP (DW_OP_reg7, 0x57)
DW_OP (DW_OP_reg8, 0x58)
DW_OP (DW_OP_reg9, 0x59)
DW_OP (DW_OP_reg10, 0x5a)
DW_OP (DW_OP_reg11, 0x5b)
DW_OP (DW_OP_reg12, 0x5c)
DW_OP (DW_OP_reg13, 0x5d)
DW_OP (DW_OP_reg14, 0x5e)
DW_OP (DW_OP_reg15, 0x5f)
DW_OP (DW_OP_reg16, 0x60)
DW_OP (DW_OP_reg17, 0x61)
DW_OP (DW_OP_reg18, 0x62)
DW_OP (DW_OP_reg19, 0x63)
DW_OP (DW_OP_reg20, 0x64)
DW_OP (DW_OP_reg21, 0x65)
DW_OP (DW_OP_reg22, 0x66)
DW_OP (DW_OP_reg23, 0x67)
DW_OP (DW_OP_reg24, 0x68)
DW_OP (DW_OP_reg25, 0x69)
DW_OP (DW_OP_reg26, 0x6a)
DW_OP (DW_OP_reg27, 0x6b)
DW_OP (DW_OP_reg28, 0x6c)
DW_OP (DW_OP_reg29, 0x6d)
DW_OP (DW_OP_reg30, 0x6e)
DW_OP (DW_OP_reg31, 0x6f)
DW_OP (DW_OP_breg0, 0x70)
DW_OP (DW_OP_breg1, 0x71)
DW_OP (DW_OP_breg2, 0x72)
DW_OP (DW_OP_breg3, 0x73)
DW_OP (DW_OP_breg4, 0x74)
DW_OP (DW_OP_breg5, 0x75)
DW_OP (DW_OP_breg6, 0x76)
DW_OP (DW_OP_breg7, 0x77)
DW_OP (DW_OP_breg8, 0x78)
DW_OP (DW_OP_breg9, 0x79)
DW_OP (DW_OP_breg10, 0x7a)
DW_OP (DW_OP_breg11, 0x7b)
DW_OP (DW_OP_breg12, 0x7c)
DW_OP (DW_OP_breg13, 0x7d)
DW_OP (DW_OP_breg14, 0x7e)
DW_OP (DW_OP_breg15, 0x7f)
DW_OP (DW_OP_breg16, 0x80)
DW_OP (DW_OP_breg17, 0x81)
DW_OP (DW_OP_breg18, 0x82)
DW_OP (DW_OP_breg19, 0x83)
DW_OP (DW_OP_breg20, 0x84)
DW_OP (DW_OP_breg21, 0x85)
DW_OP (DW_OP_breg22, 0x86)
DW_OP (DW_OP_breg23, 0x87)
DW_OP (DW_OP_breg24, 0x88)
DW_OP (DW_OP_breg25, 0x89)
DW_OP (DW_OP_breg26, 0x8a)
DW_OP (DW_OP_breg27, 0x8b)
DW_OP (DW_OP_breg28, 0x8c)
DW_OP (DW_OP_breg29, 0x8d)
DW_OP (DW_OP_breg30, 0x8e)
DW_OP (DW_OP_breg31, 0x8f)
DW_OP (DW_OP_regx, 0x90)
DW_OP (DW_OP_fbreg, 0x91)
DW_OP (DW_OP_bregx, 0x92)
DW_OP (DW_OP_piece, 0x93)
DW_OP (DW_OP_deref_size, 0x94)
DW_OP (DW_OP_xderef_size, 0x95)
DW_OP (DW_OP_nop, 0x96)
/* DWARF 3 extensions.  */
DW_OP (DW_OP_push_object_address, 0x97)
DW_OP (DW_OP_call2, 0x98)
DW_OP (DW_OP_call4, 0x99)
DW_OP (DW_OP_call_ref, 0x9a)
DW_OP (DW_OP_form_tls_address, 0x9b)
DW_OP (DW_OP_call_frame_cfa, 0x9c)
DW_OP (DW_OP_bit_piece, 0x9d)

/* DWARF 4 extensions.  */
DW_OP (DW_OP_implicit_value, 0x9e)
DW_OP (DW_OP_stack_value, 0x9f)

/* DWARF 5 extensions.  */
DW_OP (DW_OP_implicit_pointer, 0xa0)
DW_OP (DW_OP_addrx, 0xa1)
DW_OP (DW_OP_constx, 0xa2)
DW_OP (DW_OP_entry_value, 0xa3)
DW_OP (DW_OP_const_type, 0xa4)
DW_OP (DW_OP_regval_type, 0xa5)
DW_OP (DW_OP_deref_type, 0xa6)
DW_OP (DW_OP_xderef_type, 0xa7)
DW_OP (DW_OP_convert, 0xa8)
DW_OP (DW_OP_reinterpret, 0xa9)

DW_OP_DUP (DW_OP_lo_user, 0xe0)	/* Implementation-defined range start.  */
DW_OP_DUP (DW_OP_hi_user, 0xff)	/* Implementation-defined range end.  */

/* GNU extensions.  */
DW_OP (DW_OP_GNU_push_tls_address, 0xe0)
/* The following is for marking variables that are uninitialized.  */
DW_OP (DW_OP_GNU_uninit, 0xf0)
DW_OP (DW_OP_GNU_encoded_addr, 0xf1)
/* The GNU implicit pointer extension.
   See http://www.dwarfstd.org/ShowIssue.php?issue=100831.1&type=open .  */
DW_OP (DW_OP_GNU_implicit_pointer, 0xf2)
/* The GNU entry value extension.
   See http://www.dwarfstd.org/ShowIssue.php?issue=100909.1&type=open .  */
DW_OP (DW_OP_GNU_entry_value, 0xf3)
/* The GNU typed stack extension.
   See http://www.dwarfstd.org/doc/040408.1.html .  */
DW_OP (DW_OP_GNU_const_type, 0xf4)
DW_OP (DW_OP_GNU_regval_type, 0xf5)
DW_OP (DW_OP_GNU_deref_type, 0xf6)
DW_OP (DW_OP_GNU_convert, 0xf7)
DW_OP (DW_OP_GNU_reinterpret, 0xf9)
/* The GNU parameter ref extension.  */
DW_OP (DW_OP_GNU_parameter_ref, 0xfa)
/* Extensions for Fission.  See http://gcc.gnu.org/wiki/DebugFission.  */
DW_OP (DW_OP_GNU_addr_index, 0xfb)
DW_OP (DW_OP_GNU_const_index, 0xfc)
/* The GNU variable value extension.
   See http://dwarfstd.org/ShowIssue.php?issue=161109.2 . */
DW_OP (DW_OP_GNU_variable_value, 0xfd)
/* HP extensions.  */
DW_OP_DUP (DW_OP_HP_unknown, 0xe0) /* Ouch, the same as GNU_push_tls_address.  */
DW_OP (DW_OP_HP_is_value, 0xe1)
DW_OP (DW_OP_HP_fltconst4, 0xe2)
DW_OP (DW_OP_HP_fltconst8, 0xe3)
DW_OP (DW_OP_HP_mod_range, 0xe4)
DW_OP (DW_OP_HP_unmod_range, 0xe5)
DW_OP (DW_OP_HP_tls, 0xe6)
/* PGI (STMicroelectronics) extensions.  */
DW_OP (DW_OP_PGI_omp_thread_num, 0xf8)
/* AARCH64 extensions.
   DW_OP_AARCH64_operation takes one mandatory unsigned LEB128 operand.
   Bits[6:0] of this operand is the action code, all others bits are initialized
   to 0 except explicitly documented for one action.  Please refer AArch64 DWARF
   ABI documentation for details.  */
DW_OP (DW_OP_AARCH64_operation, 0xea)
DW_END_OP

//////////////////////////////////////////////////////////////////////////////////////////
DW_FIRST_CFA (DW_CFA_advance_loc, 0x40)
DW_CFA (DW_CFA_offset, 0x80)
DW_CFA (DW_CFA_restore, 0xc0)
DW_CFA (DW_CFA_nop, 0x00)
DW_CFA (DW_CFA_set_loc, 0x01)
DW_CFA (DW_CFA_advance_loc1, 0x02)
DW_CFA (DW_CFA_advance_loc2, 0x03)
DW_CFA (DW_CFA_advance_loc4, 0x04)
DW_CFA (DW_CFA_offset_extended, 0x05)
DW_CFA (DW_CFA_restore_extended, 0x06)
DW_CFA (DW_CFA_undefined, 0x07)
DW_CFA (DW_CFA_same_value, 0x08)
DW_CFA (DW_CFA_register, 0x09)
DW_CFA (DW_CFA_remember_state, 0x0a)
DW_CFA (DW_CFA_restore_state, 0x0b)
DW_CFA (DW_CFA_def_cfa, 0x0c)
DW_CFA (DW_CFA_def_cfa_register, 0x0d)
DW_CFA (DW_CFA_def_cfa_offset, 0x0e)
/* DWARF 3.  */
DW_CFA (DW_CFA_def_cfa_expression, 0x0f)
DW_CFA (DW_CFA_expression, 0x10)
DW_CFA (DW_CFA_offset_extended_sf, 0x11)
DW_CFA (DW_CFA_def_cfa_sf, 0x12)
DW_CFA (DW_CFA_def_cfa_offset_sf, 0x13)
DW_CFA (DW_CFA_val_offset, 0x14)
DW_CFA (DW_CFA_val_offset_sf, 0x15)
DW_CFA (DW_CFA_val_expression, 0x16)

DW_CFA (DW_CFA_lo_user, 0x1c)
DW_CFA (DW_CFA_hi_user, 0x3f)

/* SGI/MIPS specific.  */
DW_CFA (DW_CFA_MIPS_advance_loc8, 0x1d)
/* GNU extensions.
   NOTE: DW_CFA_GNU_window_save is multiplexed on Sparc and AArch64.  */
DW_CFA (DW_CFA_GNU_window_save, 0x2d)
DW_CFA_DUP (DW_CFA_AARCH64_negate_ra_state, 0x2d)
DW_CFA (DW_CFA_GNU_args_size, 0x2e)
DW_CFA (DW_CFA_GNU_negative_offset_extended, 0x2f)

DW_END_CFA

#undef DW_FIRST_OP
#undef DW_END_OP
#undef DW_OP
#undef DW_OP_DUP

#undef DW_FIRST_CFA
#undef DW_END_CFA
#undef DW_CFA
#undef DW_CFA_DUP
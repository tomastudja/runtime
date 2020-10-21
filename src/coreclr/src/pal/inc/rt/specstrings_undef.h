// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*

*/

#ifndef PAL_STDCPP_COMPAT
#undef __in
#undef __out
#endif // !PAL_STDCPP_COMPAT

#undef _At_
#undef _Deref_out_
#undef _Deref_out_opt_
#undef _Deref_opt_out_
#undef _Deref_opt_out_opt_
#undef _Deref_post_cap_
#undef _Deref_post_opt_cap_
#undef _Deref_post_bytecap_
#undef _Deref_post_opt_bytecap_
#undef _Deref_post_count_
#undef _Deref_post_opt_count_
#undef _Deref_post_bytecount_
#undef _Deref_post_opt_bytecount_
#undef _In_count_
#undef _In_opt_count_
#undef _In_bytecount_
#undef _In_opt_bytecount_
#undef _Out_cap_
#undef _Out_opt_cap_
#undef _Out_bytecap_
#undef _Out_opt_bytecap_
#undef _Outptr_
#undef _Outptr_result_maybenull_
#undef _Outptr_opt_
#undef _Outptr_opt_result_maybenull_
#undef _Outptr_result_z_
#undef _Outptr_opt_result_z_
#undef _Outptr_result_maybenull_z_
#undef _Outptr_opt_result_maybenull_z_
#undef _Outptr_result_nullonfailure_
#undef _Outptr_opt_result_nullonfailure_
#undef _COM_Outptr_
#undef _COM_Outptr_result_maybenull_
#undef _COM_Outptr_opt_
#undef _COM_Outptr_opt_result_maybenull_
#undef _Outptr_result_buffer_
#undef _Outptr_opt_result_buffer_
#undef _Outptr_result_buffer_to_
#undef _Outptr_opt_result_buffer_to_
#undef _Outptr_result_buffer_all_
#undef _Outptr_opt_result_buffer_all_
#undef _Outptr_result_buffer_maybenull_
#undef _Outptr_opt_result_buffer_maybenull_
#undef _Outptr_result_buffer_to_maybenull_
#undef _Outptr_opt_result_buffer_to_maybenull_
#undef _Outptr_result_buffer_all_maybenull_
#undef _Outptr_opt_result_buffer_all_maybenull_
#undef _Outptr_result_bytebuffer_
#undef _Outptr_opt_result_bytebuffer_
#undef _Outptr_result_bytebuffer_to_
#undef _Outptr_opt_result_bytebuffer_to_
#undef _Outptr_result_bytebuffer_all_
#undef _Outptr_opt_result_bytebuffer_all_
#undef _Outptr_result_bytebuffer_maybenull_
#undef _Outptr_opt_result_bytebuffer_maybenull_
#undef _Outptr_result_bytebuffer_to_maybenull_
#undef _Outptr_opt_result_bytebuffer_to_maybenull_
#undef _Outptr_result_bytebuffer_all_maybenull_
#undef _Outptr_opt_result_bytebuffer_all_maybenull_
#undef _When_
#undef __allocator
#undef __analysis_assert
#undef __analysis_assume
#undef __analysis_assume_nullterminated
#undef __analysis_hint
#undef __assume_bound
#undef __assume_validated
#undef __bcount
#undef __bcount_opt
#undef __blocksOn
#undef __bound
#undef __byte_readableTo
#undef __byte_writableTo
#undef __callback
#undef __checkReturn
#undef __class_code_content
#undef __control_entrypoint
#undef __data_entrypoint
#undef __deallocate
#undef __deallocate_opt
#undef __deref
#undef __deref_bcount
#undef __deref_bcount_opt
#undef __deref_ecount
#undef __deref_ecount_opt
#undef __deref_in
#undef __deref_in_bcount
#undef __deref_in_bcount_opt
#undef __deref_in_ecount
#undef __deref_in_ecount_opt
#undef __deref_in_ecount_iterator
#undef __deref_in_opt
#undef __deref_in_opt_out
#undef __deref_in_range
#undef __deref_in_xcount
#undef __deref_in_xcount_opt
#undef __deref_inout
#undef __deref_inout_bcount
#undef __deref_inout_bcount_full
#undef __deref_inout_bcount_full_opt
#undef __deref_inout_bcount_nz
#undef __deref_inout_bcount_nz_opt
#undef __deref_inout_bcount_opt
#undef __deref_inout_bcount_part
#undef __deref_inout_bcount_part_opt
#undef __deref_inout_bcount_z
#undef __deref_inout_bcount_z_opt
#undef __deref_inout_ecount
#undef __deref_inout_ecount_full
#undef __deref_inout_ecount_full_opt
#undef __deref_inout_ecount_nz
#undef __deref_inout_ecount_nz_opt
#undef __deref_inout_ecount_opt
#undef __deref_inout_ecount_part
#undef __deref_inout_ecount_part_opt
#undef __deref_inout_ecount_z
#undef __deref_inout_ecount_z_opt
#undef __deref_inout_ecount_iterator
#undef __deref_inout_nz
#undef __deref_inout_nz_opt
#undef __deref_inout_opt
#undef __deref_inout_range
#undef __deref_inout_xcount
#undef __deref_inout_xcount_full
#undef __deref_inout_xcount_full_opt
#undef __deref_inout_xcount_opt
#undef __deref_inout_xcount_part
#undef __deref_inout_xcount_part_opt
#undef __deref_inout_z
#undef __deref_inout_z_opt
#undef __deref_nonvolatile
#undef __deref_opt_bcount
#undef __deref_opt_bcount_opt
#undef __deref_opt_ecount
#undef __deref_opt_ecount_opt
#undef __deref_opt_in
#undef __deref_opt_in_bcount
#undef __deref_opt_in_bcount_opt
#undef __deref_opt_in_ecount
#undef __deref_opt_in_ecount_opt
#undef __deref_opt_in_opt
#undef __deref_opt_in_xcount
#undef __deref_opt_in_xcount_opt
#undef __deref_opt_inout
#undef __deref_opt_inout_bcount
#undef __deref_opt_inout_bcount_full
#undef __deref_opt_inout_bcount_full_opt
#undef __deref_opt_inout_bcount_nz
#undef __deref_opt_inout_bcount_nz_opt
#undef __deref_opt_inout_bcount_opt
#undef __deref_opt_inout_bcount_part
#undef __deref_opt_inout_bcount_part_opt
#undef __deref_opt_inout_bcount_z
#undef __deref_opt_inout_bcount_z_opt
#undef __deref_opt_inout_ecount
#undef __deref_opt_inout_ecount_full
#undef __deref_opt_inout_ecount_full_opt
#undef __deref_opt_inout_ecount_nz
#undef __deref_opt_inout_ecount_nz_opt
#undef __deref_opt_inout_ecount_opt
#undef __deref_opt_inout_ecount_part
#undef __deref_opt_inout_ecount_part_opt
#undef __deref_opt_inout_ecount_z
#undef __deref_opt_inout_ecount_z_opt
#undef __deref_opt_inout_nz
#undef __deref_opt_inout_nz_opt
#undef __deref_opt_inout_opt
#undef __deref_opt_inout_xcount
#undef __deref_opt_inout_xcount_full
#undef __deref_opt_inout_xcount_full_opt
#undef __deref_opt_inout_xcount_opt
#undef __deref_opt_inout_xcount_part
#undef __deref_opt_inout_xcount_part_opt
#undef __deref_opt_inout_z
#undef __deref_opt_inout_z_opt
#undef __deref_opt_out
#undef __deref_opt_out_bcount
#undef __deref_opt_out_bcount_full
#undef __deref_opt_out_bcount_full_opt
#undef __deref_opt_out_bcount_nz_opt
#undef __deref_opt_out_bcount_opt
#undef __deref_opt_out_bcount_part
#undef __deref_opt_out_bcount_part_opt
#undef __deref_opt_out_bcount_z_opt
#undef __deref_opt_out_ecount
#undef __deref_opt_out_ecount_full
#undef __deref_opt_out_ecount_full_opt
#undef __deref_opt_out_ecount_nz_opt
#undef __deref_opt_out_ecount_opt
#undef __deref_opt_out_ecount_part
#undef __deref_opt_out_ecount_part_opt
#undef __deref_opt_out_ecount_z_opt
#undef __deref_opt_out_nz_opt
#undef __deref_opt_out_opt
#undef __deref_opt_out_xcount
#undef __deref_opt_out_xcount_full
#undef __deref_opt_out_xcount_full_opt
#undef __deref_opt_out_xcount_opt
#undef __deref_opt_out_xcount_part
#undef __deref_opt_out_xcount_part_opt
#undef __deref_opt_out_z_opt
#undef __deref_opt_xcount
#undef __deref_opt_xcount_opt
#undef __deref_out
#undef __deref_out_bcount
#undef __deref_out_bcount_full
#undef __deref_out_bcount_full_opt
#undef __deref_out_bcount_nz
#undef __deref_out_bcount_nz_opt
#undef __deref_out_bcount_opt
#undef __deref_out_bcount_part
#undef __deref_out_bcount_part_opt
#undef __deref_out_bcount_z
#undef __deref_out_bcount_z_opt
#undef __deref_out_bound
#undef __deref_out_ecount
#undef __deref_out_ecount_full
#undef __deref_out_ecount_full_opt
#undef __deref_out_ecount_iterator
#undef __deref_out_ecount_nz
#undef __deref_out_ecount_nz_opt
#undef __deref_out_ecount_opt
#undef __deref_out_ecount_part
#undef __deref_out_ecount_part_opt
#undef __deref_out_ecount_z
#undef __deref_out_ecount_z_opt
#undef __deref_out_nz
#undef __deref_out_nz_opt
#undef __deref_out_opt
#undef __deref_out_range
#undef __deref_out_range
#undef __deref_out_xcount
#undef __deref_out_xcount
#undef __deref_out_xcount_full
#undef __deref_out_xcount_full_opt
#undef __deref_out_xcount_opt
#undef __deref_out_xcount_part
#undef __deref_out_xcount_part_opt
#undef __deref_out_z
#undef __deref_out_z_opt
#undef __deref_realloc_bcount
#undef __deref_volatile
#undef __deref_xcount
#undef __deref_xcount_opt
#undef __ecount
#undef __ecount_opt
#undef __elem_readableTo
#undef __elem_writableTo
#undef __encoded_array
#undef __encoded_pointer
#undef __exceptthat
#undef __fallthrough
#undef __field_bcount
#undef __field_bcount_full
#undef __field_bcount_full_opt
#undef __field_bcount_opt
#undef __field_bcount_part
#undef __field_bcount_part_opt
#undef __field_data_source
#undef __field_ecount
#undef __field_ecount_full
#undef __field_ecount_full_opt
#undef __field_ecount_opt
#undef __field_ecount_part
#undef __field_ecount_part_opt
#undef __field_encoded_array
#undef __field_encoded_pointer
#undef __field_nullterminated
#undef __field_range
#undef __field_xcount
#undef __field_xcount_full
#undef __field_xcount_full_opt
#undef __field_xcount_opt
#undef __field_xcount_part
#undef __field_xcount_part_opt
#undef __file_parser
#undef __file_parser_class
#undef __file_parser_library
#undef __format_string
#undef __format_string
#undef __gdi_entry
#undef __in_awcount
#undef __in_bcount
#undef __in_bcount_nz
#undef __in_bcount_nz_opt
#undef __in_bcount_opt
#undef __in_bcount_z
#undef __in_bcount_z_opt
#undef __in_bound
#undef __in_data_source
#undef __in_ecount
#undef __in_ecount_nz
#undef __in_ecount_nz_opt
#undef __in_ecount_opt
#undef __in_ecount_z
#undef __in_ecount_z_opt
#undef __in_nz
#undef __in_nz_opt
#undef __in_opt
#undef __in_range
#undef __in_xcount
#undef __in_xcount_opt
#undef __in_z
#undef __in_z_opt
#undef __inexpressible_readableTo
#undef __inexpressible_writableTo
#undef __inner_adt_add_prop
#undef __inner_adt_prop
#undef __inner_adt_remove_prop
#undef __inner_adt_transfer_prop
#undef __inner_adt_type_props
#undef __inner_analysis_assume_nulltermianted_dec
#undef __inner_analysis_assume_nullterminated
#undef __inner_assume_bound
#undef __inner_assume_bound_dec
#undef __inner_assume_validated
#undef __inner_assume_validated_dec
#undef __inner_blocksOn
#undef __inner_bound
#undef __inner_callback
#undef __inner_checkReturn
#undef __inner_compname_props
#undef __inner_control_entrypoint
#undef __inner_data_entrypoint
#undef __inner_data_source
#undef __inner_encoded
#undef __inner_nonvolatile
#undef __inner_out_validated
#undef __inner_override
#undef __inner_possibly_notnullterminated
#undef __inner_range
#undef __inner_success
#undef __inner_transfer
#undef __inner_typefix
#undef __inner_volatile
#undef __inout
#undef __inout_bcount
#undef __inout_bcount_full
#undef __inout_bcount_full_opt
#undef __inout_bcount_nz
#undef __inout_bcount_nz_opt
#undef __inout_bcount_opt
#undef __inout_bcount_part
#undef __inout_bcount_part_opt
#undef __inout_bcount_z
#undef __inout_bcount_z_opt
#undef __inout_ecount
#undef __inout_ecount_full
#undef __inout_ecount_full_opt
#undef __inout_ecount_nz
#undef __inout_ecount_nz_opt
#undef __inout_ecount_opt
#undef __inout_ecount_part
#undef __inout_ecount_part_opt
#undef __inout_ecount_z
#undef __inout_ecount_z_opt
#undef __inout_ecount_z_opt
#undef __inout_nz
#undef __inout_nz_opt
#undef __inout_opt
#undef __inout_xcount
#undef __inout_xcount_full
#undef __inout_xcount_full_opt
#undef __inout_xcount_opt
#undef __inout_xcount_part
#undef __inout_xcount_part_opt
#undef __inout_z
#undef __inout_z_opt
#undef __kernel_entry
#undef __maybenull
#undef __maybereadonly
#undef __maybevalid
#undef __range_max
#undef __range_min
#undef __nonvolatile
#undef __notnull
#undef __notreadonly
#undef __notvalid
#undef __null
#undef __nullnullterminated
#undef __nullterminated
#undef __out_awcount
#undef __out_bcount
#undef __out_bcount_full
#undef __out_bcount_full_opt
#undef __out_bcount_nz
#undef __out_bcount_nz_opt
#undef __out_bcount_opt
#undef __out_bcount_part
#undef __out_bcount_part_opt
#undef __out_bcount_z
#undef __out_bcount_z_opt
#undef __out_bound
#undef __out_data_source
#undef __out_ecount
#undef __out_ecount_full
#undef __out_ecount_full_opt
#undef __out_ecount_nz
#undef __out_ecount_nz_opt
#undef __out_ecount_opt
#undef __out_ecount_part
#undef __out_ecount_part_opt
#undef __out_ecount_z
#undef __out_ecount_z_opt
#undef __out_has_adt_prop
#undef __out_has_type_adt_props
#undef __out_not_has_adt_prop
#undef __out_nz
#undef __out_nz_opt
#undef __out_opt
#undef __out_range
#undef __out_transfer_adt_prop
#undef __out_validated
#undef __out_xcount
#undef __out_xcount_full
#undef __out_xcount_full_opt
#undef __out_xcount_opt
#undef __out_xcount_part
#undef __out_xcount_part_opt
#undef __out_z
#undef __override
#undef __possibly_notnullterminated
#undef __post
#undef __post_invalid
#undef __postcond
#undef __post_nullnullterminated
#undef __pre
#undef __precond
#undef __range
#undef __readableTo
#undef __readonly
#undef __refparam
#undef __clr_reserved
#undef __rpc_entry
#undef __source_code_content
#undef __struct_bcount
#undef __struct_xcount
#undef __success
#undef __this_out_data_source
#undef __this_out_validated
#undef __transfer
#undef __type_has_adt_prop
#undef __typefix
#undef __valid
#undef __volatile
#undef __writableTo
#undef __xcount
#undef __xcount_opt


define ____crxdbg_globals
	if basic_functions_module.zts
		if !$tsrm_ls
			set $tsrm_ls = ts_resource_ex(0, 0)
		end
		set $crxdbg = ((crex_crxdbg_globals*) (*((void ***) $tsrm_ls))[crxdbg_globals_id-1])
	else
		set $crxdbg = crxdbg_globals
	end
end

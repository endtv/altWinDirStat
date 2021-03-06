//
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#pragma once

#include "stdafx.h"

#ifndef WDS_HWND_FUNCS_H
#define WDS_HWND_FUNCS_H

WDS_FILE_INCLUDE_MESSAGE

namespace hwnd {
	//CWnd::Invalidate():
	void InvalidateErase(_In_opt_ const HWND hWnd, _In_opt_ const bool do_we_care = true, _In_opt_z_ PCSTR source = nullptr) noexcept;
	void RedrawWindow(_In_ const HWND hWnd) noexcept;
	void ScreenToClient(_In_ const HWND hWnd, _Inout_ POINT* point) noexcept;
	}



#else
#error ass //my usual error :D
#endif

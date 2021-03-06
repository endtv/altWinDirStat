// graphview.h	- Declaration of CGraphView (the Treemap view)
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_H
#define WDS_GRAPHVIEW_H

WDS_FILE_INCLUDE_MESSAGE

#include "datastructures.h"

#include "macros_that_scare_small_children.h"
#include "ScopeGuard.h"
#include "treemap.h"
//#include "options.h"
//#include "dirstatdoc.h"
//#include "mainframe.h"
#include "LOGICAL_FOCUS_enum.h"
//#include "windirstat.h"//For CDirstatApp* m_appptr

class CDirstatDoc;
class CGraphView;
class CMainFrame;
class CDirstatApp;


#ifdef DEBUG
void trace_empty_view_graphview( ) noexcept;
void trace_call_onidle( ) noexcept;
void trace_mouse_left( ) noexcept;
void trace_focused_mouspos( _In_ const LONG x, _In_ const LONG y, _In_z_ PCWSTR const path ) noexcept;
#endif


// CGraphView. The treemap window.
class CGraphView final : public CView {
public:

	//NOTE TO SELF: cannot init SIZE structs like `= { 0, 0 };` here, as compiler doesn't like using "list" init in NSDMI.
	//TODO: use plain old SIZE struct
	SIZE              m_size = { };				// Current size of view
	bool              m_recalculationSuspended : 1; // True while the user is resizing the window.	
	//C4820: 'CGraphView' : '3' bytes padding added after data member 'CGraphView::m_showTreemap'
	bool              m_showTreemap            : 1; // False, if the user switched off the treemap (by F9).

protected:
	CTreemap          m_treemap;				// Treemap generator
	CBitmap           m_bitmap;				// Cached view. If m_hObject is NULL, the view must be recalculated.
	//HBITMAP           m_bitmapC;
	SIZE              m_dimmedSize = { };			// Size of bitmap m_dimmed
	CBitmap           m_dimmed;				// Dimmed view. Used during refresh to avoid the ooops-effect.
	UINT_PTR          m_timer = 0u;				// We need a timer to realize when the mouse left our window.
	CMainFrame* const m_frameptr;
	CDirstatApp*      m_appptr;

	
protected:
	CGraphView( );
	/*
#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

----------------------------------------

public:
	static const CRuntimeClass classCGraphView;
	virtual CRuntimeClass* GetRuntimeClass( ) const;
	static CObject* PASCAL CreateObject( );

	*/

public:
	static const CRuntimeClass classCGraphView;
	
	virtual CRuntimeClass* GetRuntimeClass( ) const noexcept final {
		return const_cast<CRuntimeClass*>( &CGraphView::classCGraphView );
		}

	static CObject* PASCAL CreateObject( ) {
		return new CGraphView;
		}

	//DECLARE_DYNCREATE(CGraphView)

public:
	//virtual ~CGraphView();

	DISALLOW_COPY_AND_ASSIGN( CGraphView );

	void SuspendRecalculation(const bool suspend) noexcept;

	void DrawEmptyView( ) noexcept {
		CClientDC dc( this );
		DrawEmptyView( dc.m_hDC );
		}

protected:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs ) override final {
		// We don't want a background brush
		VERIFY( CView::PreCreateWindow( cs ) ); // this registers a wndclass
	
		WNDCLASS wc;
		VERIFY( ::GetClassInfoW( ::AfxGetInstanceHandle( ), cs.lpszClass, &wc ) );
		wc.hbrBackground = { NULL };
		wc.lpszClassName = _T( "windirstat_graphview_class" );
		//RegisterClassW function: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassw
		//If the function succeeds, the return value is a class atom that uniquely identifies the class being registered. 
		//If the function fails, the return value is zero. To get extended error information, call GetLastError.
		const ATOM register_class_result = ::RegisterClassW(&wc);
		if (register_class_result == 0) {
			displayWindowsMsgBoxWithError();
			std::terminate();
		}

		//A pointer to a null-terminated string or is an atom.
			//If this parameter is an atom, it must be a class atom created by a previous call to the RegisterClass or RegisterClassEx function.
			//The atom must be in the low-order word of lpszClassName; the high-order word must be zero.

		//If lpszClassName is a string, it specifies the window class name.
			//The class name can be any name registered with RegisterClass or RegisterClassEx, or any of the predefined control - class names.
		cs.lpszClass = reinterpret_cast<PCWSTR>( register_class_result );
		ASSERT(HIWORD(cs.lpszClass) == 0);
		TRACE(L"RegisterClassW result: %u\r\n", static_cast<unsigned>(register_class_result));

		return TRUE;

		}
	
	virtual void OnInitialUpdate( ) override final {
		/*
		void CView::OnInitialUpdate()
		{
			OnUpdate(NULL, 0, NULL);        // initial update
		}
		*/
		//OnUpdate(NULL, 0, NULL) calls CGraphView::OnUpdate
		CView::OnInitialUpdate( );
		}
	
	virtual void OnDraw( CDC* pScreen_Device_Context ) override final;
	
	//this is just a comparison, it should be inlined.
	inline bool IsDrawn( ) const noexcept {
		return ( m_bitmap.m_hObject != NULL );
		}

	void Inactivate( ) noexcept {
		if ( m_bitmap.m_hObject != nullptr ) {
			m_dimmed.Detach( );
			VERIFY( m_dimmed.Attach( m_bitmap.Detach( ) ) );
			m_dimmedSize = m_size;
			}
		}
	
	void reset_timer_if_zero( ) noexcept {
		if ( m_timer == 0 ) {
#ifdef DEBUG
			trace_mouse_left( );
#endif
			m_timer = SetTimer( 4711, 100, nullptr);//TODO: figure out what the hell this does.//if value is increased ( argument 2 ), program execution will take longer to reach `TRACE( _T( "Mouse has left the tree map area!\r\n" ) );` after mouse has left tree map area.
			}
		}

	void EmptyView( ) noexcept {
		if ( m_bitmap.m_hObject != nullptr ) {
			m_bitmap.DeleteObject( );
			}
		if ( m_dimmed.m_hObject != nullptr ) {
			m_dimmed.DeleteObject( );
			}
		}

	void cause_OnIdle_to_be_called_once( ) const noexcept {
		// Cause OnIdle() to be called once.
#ifdef DEBUG
		trace_call_onidle( );
#endif
		//PostThreadMessageW function: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postthreadmessagew
		//If the function succeeds, the return value is nonzero.
		//If the function fails, the return value is zero. To get extended error information, call GetLastError. GetLastError returns ERROR_INVALID_THREAD_ID if idThread is not a valid thread identifier, or if the thread specified by idThread does not have a message queue. GetLastError returns ERROR_NOT_ENOUGH_QUOTA when the message limit is hit.
		const BOOL post_message_response = ::PostThreadMessageW( ::GetCurrentThreadId( ), WM_NULL, 0, 0 );
		if (post_message_response == 0) {
			displayWindowsMsgBoxWithError();
			std::terminate();
		}
		}

public:
	
	//Keeping RenderHighlightRectangle in the implementation file means that we don't need to include options.h in the header.
	void RenderHighlightRectangle( _In_ const HDC screen_device_context, _In_ RECT rc_ ) const noexcept;

protected:
	void DrawEmptyView( _In_ const HDC Screen_Device_Context ) noexcept {
#ifdef DEBUG
		trace_empty_view_graphview( );
#endif
		constexpr const COLORREF gray = RGB( 160, 160, 160 );
		CGraphView::Inactivate( );

		RECT rc = { 0 };

		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
		//If the window handle identifies an existing window, the return value is nonzero.
		//If the window handle does not identify an existing window, the return value is zero.
		ASSERT( ::IsWindow( m_hWnd ) );

		//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
		//Return value: If the function succeeds, the return value is nonzero.
		//If the function fails, the return value is zero.
		//To get extended error information, call GetLastError.
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );

		if ( m_dimmed.m_hObject == nullptr ) {
			//return pScreen_Device_Context->FillSolidRect( &rc, gray );
			fill_solid_RECT( Screen_Device_Context, &rc, gray );
			return;
			}
		//CDC offscreen_buffer;
		//VERIFY( offscreen_buffer.CreateCompatibleDC( pScreen_Device_Context ) );
		
		const HDC offscreen_buffer = gdi::CreateCompatibleDeviceContext( Screen_Device_Context );
		auto guard = WDS_SCOPEGUARD_INSTANCE([&] {
			gdi::DeleteDeviceContext(offscreen_buffer);
			});
		SelectObject_wrapper sobmp( offscreen_buffer, m_dimmed.m_hObject );

		VERIFY(::BitBlt(Screen_Device_Context, rc.left, rc.top, m_dimmedSize.cx, m_dimmedSize.cy, offscreen_buffer, 0, 0, SRCCOPY));
		//VERIFY( pScreen_Device_Context->BitBlt( rc.left, rc.top, m_dimmedSize.cx, m_dimmedSize.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

		if ( ( rc.right - rc.left ) > m_dimmedSize.cx ) {
			RECT r = rc;
			r.left = r.left + m_dimmedSize.cx;
			//pScreen_Device_Context->FillSolidRect( &r, gray );
			fill_solid_RECT( Screen_Device_Context, &r, gray );
			}

		if ( ( rc.bottom - rc.top ) > m_dimmedSize.cy ) {
			RECT r = rc;
			r.top = r.top + m_dimmedSize.cy;
			//pScreen_Device_Context->FillSolidRect( &r, gray );
			fill_solid_RECT( Screen_Device_Context, &r, gray );
			}
		//VERIFY( dcmem.DeleteDC( ) );
		}


	//Keeping DrawHighlightExtension in the implementation file means that we don't need to include options.h in the header.
	void DrawHighlightExtension( _In_ const HDC screen_device_context ) const noexcept;


	void RecurseHighlightExtension( _In_ const HDC screen_device_context, _In_ const CTreeListItem& item, _In_ const std::wstring& ext ) const noexcept;

	void DrawSelection( _In_ const HDC screen_device_context ) const noexcept;
	
	void DoDraw( _Inout_ RECT* const rc, _In_ HDC const offscreen_buffer_device_context, _In_ HDC const offscreen_buffer_attribute_device_context) noexcept;
	
	
	void DrawViewNotEmpty( _In_ const HDC Screen_Device_Context ) noexcept {
		//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
		//If the window handle identifies an existing window, the return value is nonzero.
		//If the window handle does not identify an existing window, the return value is zero.
		ASSERT( ::IsWindow( m_hWnd ) );


		RECT rc = { 0 };

		//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
		//Return value: If the function succeeds, the return value is nonzero.
		//If the function fails, the return value is zero.
		//To get extended error information, call GetLastError.
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );

		//CDC offscreen_buffer;
		//VERIFY( offscreen_buffer.CreateCompatibleDC( Screen_Device_Context ) );
		const HDC offscreen_buffer = gdi::CreateCompatibleDeviceContext( Screen_Device_Context );
		auto guard = WDS_SCOPEGUARD_INSTANCE([&] {
			gdi::DeleteDeviceContext(offscreen_buffer);
			});

		if ( !CGraphView::IsDrawn( ) ) {
			/*
			From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afxwin1.inl:216:
				_AFXWIN_INLINE BOOL CBitmap::CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight)
				{ return Attach(::CreateCompatibleBitmap(pDC->m_hDC, nWidth, nHeight)); }
			//_AFXWIN_INLINE BOOL CBitmap::CreateCompatibleBitmap(CDC* pDC, int nWidth, int nHeight)
			//{ return Attach(::CreateCompatibleBitmap(pDC->m_hDC, nWidth, nHeight)); }
			*/

			VERIFY(m_bitmap.Attach(gdi::CreateCompatibleBitmap(Screen_Device_Context, m_size.cx, m_size.cy)));
			//VERIFY(m_bitmap.CreateCompatibleBitmap(Screen_Device_Context, m_size.cx, m_size.cy));


			// https://docs.microsoft.com/en-us/cpp/mfc/reference/cdc-class?view=vs-2019#m_hdc
			// "By default, m_hDC is equal to m_hAttribDC, the other device context wrapped by CDC"
			//ASSERT(offscreen_buffer.m_hDC == offscreen_buffer.m_hAttribDC);
			DoDraw( &rc, offscreen_buffer, offscreen_buffer);
			}

		SelectObject_wrapper sobmp2( offscreen_buffer, m_bitmap.m_hObject );
		/*
		_AFXWIN_INLINE BOOL CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
			int xSrc, int ySrc, DWORD dwRop)
			{ ASSERT(m_hDC != NULL); return ::BitBlt(m_hDC, x, y, nWidth, nHeight,
				pSrcDC->GetSafeHdc(), xSrc, ySrc, dwRop); }

		*/
		VERIFY(::BitBlt(Screen_Device_Context, 0, 0, m_size.cx, m_size.cy, offscreen_buffer, 0, 0, SRCCOPY));
		//VERIFY( Screen_Device_Context->BitBlt( 0, 0, m_size.cx, m_size.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

		DrawHighlights( Screen_Device_Context);
		//VERIFY( dcmem.DeleteDC( ) );
		}

public:

protected:
	_Pre_satisfies_( item.m_child_info.m_child_info_ptr != NULL )
	void RecurseHighlightChildren( _In_ const HDC screen_device_context, _In_ const CTreeListItem& item, _In_ const std::wstring& ext ) const;

	//Keeping DrawHighlights in the implementation file means that we don't need to include windirstat.h in the header.
	void DrawHighlights( _In_ const HDC Screen_Device_Context ) const noexcept;


	/*
#define DECLARE_MESSAGE_MAP() \
protected: \
	static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
	virtual const AFX_MSGMAP* GetMessageMap() const; \
-------------------------------------------------------
	*/
protected:
	static const AFX_MSGMAP* PASCAL GetThisMessageMap( );
	
	virtual const AFX_MSGMAP* GetMessageMap( ) const final {
		return GetThisMessageMap( );
		}


	afx_msg void OnSize( UINT nType, INT cx, INT cy ) {
		CWnd::OnSize( nType, cx, cy );
		SIZE sz = { cx, cy };
		if ( ( sz.cx != m_size.cx ) || ( sz.cy != m_size.cy ) ) {
			CGraphView::Inactivate( );
			m_size = sz;
			}
		}


	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	afx_msg void OnSetFocus( CWnd* /*pOldWnd*/ );

	//Keeping OnUpdate in the implementation file means that we don't need to include dirstatdoc.h in the header.
	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final;
	
	afx_msg void OnContextMenu( CWnd* /*pWnd*/, CPoint ptscreen );

	afx_msg void OnMouseMove( UINT /*nFlags*/, CPoint point );

	afx_msg void OnDestroy( ) {
		if ( m_timer != 0 ) {
			VERIFY( CWnd::KillTimer( m_timer ) );
			}
		m_timer = 0;
		CView::OnDestroy( );
		}
	
	//Keeping OnTimer in the implementation file means that we don't need to include mainframe.h in the header.
	afx_msg void OnTimer( UINT_PTR /*nIDEvent*/ );
	};

#else

#endif

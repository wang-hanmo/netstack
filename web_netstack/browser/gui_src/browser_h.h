

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for browser.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __browser_h_h__
#define __browser_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __Ibrowser_FWD_DEFINED__
#define __Ibrowser_FWD_DEFINED__
typedef interface Ibrowser Ibrowser;

#endif 	/* __Ibrowser_FWD_DEFINED__ */


#ifndef __browser_FWD_DEFINED__
#define __browser_FWD_DEFINED__

#ifdef __cplusplus
typedef class browser browser;
#else
typedef struct browser browser;
#endif /* __cplusplus */

#endif 	/* __browser_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __browser_LIBRARY_DEFINED__
#define __browser_LIBRARY_DEFINED__

/* library browser */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_browser;

#ifndef __Ibrowser_DISPINTERFACE_DEFINED__
#define __Ibrowser_DISPINTERFACE_DEFINED__

/* dispinterface Ibrowser */
/* [uuid] */ 


EXTERN_C const IID DIID_Ibrowser;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("ebb0bf12-bade-4363-a9fc-a5f810576c9c")
    Ibrowser : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IbrowserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Ibrowser * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Ibrowser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Ibrowser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Ibrowser * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Ibrowser * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Ibrowser * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Ibrowser * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IbrowserVtbl;

    interface Ibrowser
    {
        CONST_VTBL struct IbrowserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Ibrowser_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define Ibrowser_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define Ibrowser_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define Ibrowser_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define Ibrowser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define Ibrowser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define Ibrowser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __Ibrowser_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_browser;

#ifdef __cplusplus

class DECLSPEC_UUID("73d80c10-388a-46c1-81b5-c2930900b617")
browser;
#endif
#endif /* __browser_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



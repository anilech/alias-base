// alias-base plugin
// base search only
// 
#include <stdio.h>
#include <string.h>
#include "./dirsrv/slapi-plugin.h"

#define PLUGINNAME "alias-base"
#define PLUGINVNDR "anilech"
#define PLUGINVERS "0.1"
#define PLUGINDESC "alias plugin [base search only]"

#define MAXALIASCHAIN 8
#define MYALIASFILTER "(&(objectClass=alias)(aliasedObjectName=*))"

#ifdef _WIN32
__declspec(dllexport)
#endif

static Slapi_ComponentId *plugin_id = NULL;

// function prototypes 
int alias_base_init( Slapi_PBlock *pb );
int alias_base_srch( Slapi_PBlock *pb );

/* Description of the plug-in */
Slapi_PluginDesc srchpdesc = { PLUGINNAME, PLUGINVNDR, PLUGINVERS, PLUGINDESC };


/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
// init function

int alias_base_init( Slapi_PBlock *pb ) {

  if (
     // Specify the version of the plug-in ( "03" in this release ) 
     slapi_pblock_set( pb, SLAPI_PLUGIN_VERSION, SLAPI_PLUGIN_VERSION_03 ) != 0 ||
     // Specify the description of the plug-in 
     slapi_pblock_set( pb,SLAPI_PLUGIN_DESCRIPTION, (void *)&srchpdesc ) != 0 ||
     // Set function to call before 
     slapi_pblock_set( pb, SLAPI_PLUGIN_PRE_SEARCH_FN, (void *) alias_base_srch ) != 0  ||
     // get plugin id 
     slapi_pblock_get(pb, SLAPI_PLUGIN_IDENTITY, &plugin_id) !=0
     ) {
        slapi_log_error( SLAPI_LOG_FATAL, PLUGINNAME ,"INIT: Error registering the plug-in.\n" );
        return( -1 );
     }

  // If successful, log a message and return 0
  slapi_log_error( SLAPI_LOG_PLUGIN, PLUGINNAME ,"INIT: Plug-in successfully registered.\n" );
  return( 0 );
}

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

char *getNextDN(char *dn){

  Slapi_PBlock *pb = NULL;
  Slapi_Entry **e;

  int derefNever = LDAP_DEREF_NEVER;
  char *a[] = { "aliasedObjectName", NULL };
  char *v=NULL;

  // search dn to check if it is an alias
  pb = slapi_pblock_new();
  slapi_search_internal_set_pb(pb, dn, LDAP_SCOPE_BASE, MYALIASFILTER, a, 0, NULL, NULL, plugin_id, 0);
  slapi_pblock_set(pb, SLAPI_SEARCH_DEREF, (void *)&derefNever);
  
  if (slapi_search_internal_pb(pb) == 0) {
    slapi_pblock_get(pb, SLAPI_PLUGIN_INTOP_SEARCH_ENTRIES, &e);
    if (e != NULL && e[0] != NULL) v=slapi_entry_attr_get_charptr(e[0], a[0]); // slapi_ch_free_string() is required
  }

  slapi_free_search_results_internal(pb);
  slapi_pblock_destroy(pb);
  return v;
}


/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

int alias_base_srch( Slapi_PBlock *pb ) {
  char *s=NULL;
  int i;

  char *dn1=NULL, *dn2=NULL;

  // skip reserved operations
  if( slapi_op_reserved(pb) ) return 0;

  // skip our own requests
  if ( slapi_pblock_get( pb, SLAPI_SEARCH_STRFILTER, &s ) != 0 ) return( 0 );
  if (strcmp(s,MYALIASFILTER)==0) return 0; // got our own request, relax...

  // only base search is supported
  if ( slapi_pblock_get( pb, SLAPI_SEARCH_SCOPE, &i ) != 0 ) return(0);
  if ( i != LDAP_SCOPE_BASE ) return( 0 );

  // deref must be enabled
  if ( slapi_pblock_get( pb, SLAPI_SEARCH_DEREF, &i ) != 0 ) return( 0 );
  if ( i != LDAP_DEREF_FINDING && i != LDAP_DEREF_ALWAYS) return( 0 );
  
  // dn should be provided
  if ( slapi_pblock_get( pb, SLAPI_SEARCH_TARGET, &s ) != 0 || s == NULL || strlen(s)==0 ) return(0);

  // ok they search base with deref and we got base dn in s

  i=0; dn2=s;
  do {
    dn1=dn2;
    dn2=getNextDN(dn1);
    if (i>0 && dn2 != NULL) slapi_ch_free_string(&dn1);
  } while (dn2 != NULL && i++ <MAXALIASCHAIN);

  if (dn1 == s) return (0); // source dn is not an alias

  if (dn2 == NULL) slapi_pblock_set(pb, SLAPI_SEARCH_TARGET, dn1); // alias resolved, set new base
  else slapi_ch_free_string(&dn2); // here we hit an alias chain longer than MAXALIASCHAIN

  slapi_ch_free_string(&dn1);
  return (0);

}

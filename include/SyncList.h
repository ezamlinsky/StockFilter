/*                                                                    SyncList.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  SYNC LIST                                   #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Sync list constants                                                   //
//****************************************************************************//
# define	SYNC_COLUMNS		5				// Count of columns in sync list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	SYNC_TICKER_ID		0				// Stock ticker field id
# define	SYNC_QUOTES_ID		1				// Quotes count field id
# define	SYNC_START_ID		2				// Start date field id
# define	SYNC_END_ID			3				// End date field id
# define	SYNC_STATUS_ID		4				// Sync status field id

//============================================================================//
//      Field names                                                           //
//============================================================================//
# define	SYNC_TICKER_LABEL	"Ticker"		// Stock ticker field label
# define	SYNC_QUOTES_LABEL	"Quotes"		// Quotes count field label
# define	SYNC_START_LABEL	"Start date"	// Start date field label
# define	SYNC_END_LABEL		"End date"		// End date field label
# define	SYNC_STATUS_LABEL	"Status"		// Sync status field label

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
gboolean SyncQuotesDialog (GtkWindow *parent, GtkTreeModel *model, const gchar *fname, const gchar *tzone);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

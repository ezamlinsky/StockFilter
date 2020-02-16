/*                                                                   CheckList.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  CHECK LIST                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Check list constants                                                  //
//****************************************************************************//
# define	CHECK_COLUMNS		7				// Count of columns in check list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	CHECK_TICKER_ID		0				// Stock ticker field id
# define	CHECK_QUOTES_ID		1				// Quotes count field id
# define	CHECK_FIRST_ID		2				// First quote date field id
# define	CHECK_LAST_ID		3				// Last quote date field id
# define	CHECK_SYNC_ID		4				// Sync time field id
# define	CHECK_PRICE_ID		5				// Last quote price field id
# define	CHECK_STATUS_ID		6				// Check status field id

//============================================================================//
//      Field names                                                           //
//============================================================================//
# define	CHECK_TICKER_LABEL	"Ticker"		// Stock ticker field label
# define	CHECK_QUOTES_LABEL	"Quotes"		// Quotes count field label
# define	CHECK_FIRST_LABEL	"First quote"	// First quote date field label
# define	CHECK_LAST_LABEL	"Last quote"	// Last quote date field label
# define	CHECK_SYNC_LABEL	"Sync time"		// Sync time field label
# define	CHECK_PRICE_LABEL	"Last price"	// Last quote price field label
# define	CHECK_STATUS_LABEL	"Status"		// Check status field label

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
gboolean CheckQuotesDialog (GtkWindow *parent, GtkTreeModel *model, const gchar *fname);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

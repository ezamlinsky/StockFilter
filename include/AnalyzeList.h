/*                                                                 AnalyzeList.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 ANALYZE LIST                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Analyze list constants                                                //
//****************************************************************************//
# define	ANALYZE_COLUMNS				8				// Count of columns in analyze list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	ANALYZE_TICKER_ID			0				// Stock ticker field id
# define	ANALYZE_DATE_ID				1				// Last quote date field id
# define	ANALYZE_SYNC_ID				2				// Sync time field id
# define	ANALYZE_QUOTES_ID			3				// Quotes count field id
# define	ANALYZE_LIQUIDITY_ID		4				// Stock liquidity field id
# define	ANALYZE_VOLATILITY_ID		5				// Stock volatility field id
# define	ANALYZE_PRICE_ID			6				// Base price field id
# define	ANALYZE_STATUS_ID			7				// Analyze status field id

//============================================================================//
//      Field names                                                           //
//============================================================================//
# define	ANALYZE_TICKER_LABEL		"Ticker"		// Stock ticker field labels
# define	ANALYZE_DATE_LABEL			"Date"			// Last quote date field label
# define	ANALYZE_SYNC_LABEL			"Sync"			// Sync time field label
# define	ANALYZE_QUOTES_LABEL		"Quotes"		// Quotes count field label
# define	ANALYZE_LIQUIDITY_LABEL		"Liquidity"		// Stock liquidity field label
# define	ANALYZE_VOLATILITY_LABEL	"Volatility"	// Stock volatility field label
# define	ANALYZE_PRICE_LABEL			"Price"			// Base price field label
# define	ANALYZE_STATUS_LABEL		"Status"		// Analyze status field label

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
gboolean AnalyzeQuotesDialog (GtkWindow *parent, GtkTreeModel *model, const gchar *fname, gint count, gfloat liquidity, gfloat volatility, gfloat price);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

/*                                                                   QuoteList.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                  QUOTE LIST                                  #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>
# include	<Quotes.h>
# include	<Time.h>

//****************************************************************************//
//      Quote list constants                                                  //
//****************************************************************************//
# define	QUOTE_COLUMNS			7			// Count of columns in quote list

//============================================================================//
//      Field ids                                                             //
//============================================================================//
# define	QUOTE_DATE_ID			0			// Quote date field id
# define	QUOTE_OPEN_ID			1			// Quote open price field id
# define	QUOTE_HIGH_ID			2			// Quote high price field id
# define	QUOTE_LOW_ID			3			// Quote low price field id
# define	QUOTE_CLOSE_ID			4			// Quote close price field id
# define	QUOTE_ADJCLOSE_ID		5			// Quote adjusted close price field id
# define	QUOTE_VOLUME_ID			6			// Quote volume field id

//============================================================================//
//      Field names                                                           //
//============================================================================//
# define	QUOTE_DATE_LABEL		"Date"		// Quote date field label
# define	QUOTE_OPEN_LABEL		"Open"		// Quote open price field label
# define	QUOTE_HIGH_LABEL		"High"		// Quote high price field label
# define	QUOTE_LOW_LABEL			"Low"		// Quote low price field label
# define	QUOTE_CLOSE_LABEL		"Close"		// Quote close price field label
# define	QUOTE_ADJCLOSE_LABEL	"Adj close"	// Quote adjusted close price field label
# define	QUOTE_VOLUME_LABEL		"Volume"	// Quote volume field label

//****************************************************************************//
//      Function prototypes                                                   //
//****************************************************************************//
time_t ExtractDate (const gchar *string, GError **error);
gfloat ExtractPrice (const gchar *string, const gchar *field, GError **error);
gsize ExtractVolume (const gchar *string, GError **error);
gboolean IsQuoteCorrect (time_t date, gfloat open, gfloat high, gfloat low, gfloat close, GError **error);
gboolean OpenQuoteList (Quotes *quotes, const gchar *fname, GError **error);
gboolean SaveQuoteList (Quotes *quotes, const gchar *fname, GError **error);
gboolean ViewQuotesDialog (GtkWindow *parent, const gchar *path, const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url);
gboolean EditQuotesDialog (GtkWindow *parent, const gchar *path, const gchar *ticker, const gchar *name, const gchar *country, const gchar *sector, const gchar *industry, const gchar *url);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

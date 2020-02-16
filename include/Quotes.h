/*                                                                      Quotes.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 QUOTES CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>
# include	<Time.h>
# include	<Accumulator.h>

//****************************************************************************//
//      Quote constants                                                       //
//****************************************************************************//
# define	MIN_DATE	0x386D4380		// Min quote date to retrieve

//****************************************************************************//
//      Stock quote structure                                                 //
//****************************************************************************//
struct quote_t
{
	time_t	date;			// Quote date
	gfloat	open;			// Quote open price
	gfloat	high;			// Quote high price
	gfloat	low;			// Quote low price
	gfloat	close;			// Quote close price
	gfloat	adjclose;		// Quote adjusted close price
	gsize	volume;			// Quote volume
};

//****************************************************************************//
//      Quote list structure                                                  //
//****************************************************************************//
struct QuoteList
{
	quote_t	*array;			// Quotes array
	gsize	size;			// Size of quotes array
};

//****************************************************************************//
//      Quotes class                                                          //
//****************************************************************************//
class Quotes
{
private:
	quote_t	*array;			// Quotes array
	gsize	size;			// Size of quotes array
	time_t	synctime;		// Quotes sync time

public:

	// Constructor and destructor
	Quotes (void);
	~Quotes (void);

	// Create new quote list
	void NewList (time_t stime);

	// Quote list opening and saving
	gboolean OpenList (const gchar *fname, GError **error);
	gboolean SaveList (const gchar *fname, GError **error);

	// Quote list importing and exporting
	gboolean ImportList (const gchar *fname, GError **error);
	gboolean ExportList (const gchar *fname, GError **error);

	// Quote list operations
	gboolean AddQuotes (QuoteList newlist, time_t stime, GError **error);
	gboolean SetQuotes (GtkListStore *list, GError **error);
	GtkListStore* GetQuotes (void) const;

	// Quote list
	QuoteList GetQuoteList (void) const;

	// Quote properties
	gint GetCount (void) const;
	time_t GetSyncTime (void) const;
	time_t GetFirstDate (void) const;
	time_t GetLastDate (void) const;
	gfloat GetLastPrice (void) const;
	gsize GetLiquidity (gsize count) const;
	gfloat GetVolatility (gsize count) const;
};

//****************************************************************************//
//      Global functions                                                      //
//****************************************************************************//
gsize ExtractQuotes (const gchar *buffer, Accumulator *accumulator, GError **error);
QuoteList CheckQuotes (const quote_t *array, gsize size, GError **error);
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

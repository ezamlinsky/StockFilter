/*                                                                      Client.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 CLIENT CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<curl/curl.h>
# include	<gtk/gtk.h>
# include	<Time.h>

//****************************************************************************//
//      Client class                                                          //
//****************************************************************************//
class Client
{
private:
	quote_t	*array;			// Quotes array
	gsize	size;			// Size of quotes array
	CURL	*handle;		// CURL handle

public:

	// Constructor and destructor
	Client (void);
	~Client (void);

	// Client initialization
	gboolean Init (GError **error);

	// Check for quote splits
	gboolean CheckSplits (const gchar *ticker, time_t start, time_t end, GError **error);

	// Request quotes from quote server
	gboolean GetQuotes (const gchar *ticker, time_t start, time_t end, GError **error);

	// Quote list
	QuoteList GetQuoteList (void) const;

	// Quote properties
	gint GetCount (void) const;
	time_t GetFirstDate (void) const;
	time_t GetLastDate (void) const;
};
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

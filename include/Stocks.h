/*                                                                      Stocks.h
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 STOCKS CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# pragma	once
# include	<gtk/gtk.h>

//****************************************************************************//
//      Stocks class                                                          //
//****************************************************************************//
class Stocks
{
private:
	GtkListStore	*list;			// Stock list
	gchar			*timezone;		// Stock time zone

public:

	// Constructor and destructor
	Stocks (void);
	~Stocks (void);

	// Create new stock list
	void NewList (const gchar *tzone);

	// Clear stock list
	void ClearList (void);

	// Stock list opening and saving
	gboolean OpenList (const gchar *fname, GError **error);
	gboolean SaveList (const gchar *fname, GError **error);

	// Stock list importing and exporting
	gboolean ImportList (const gchar *fname, GError **error);
	gboolean ExportList (const gchar *fname, GError **error);

	// Stock properties
	GtkListStore* GetStockList (void) const;
	gchar* GetTimeZone (void) const;
};
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

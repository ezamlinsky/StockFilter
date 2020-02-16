/*                                                                    Stocks.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 STOCKS CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<StockList.h>
# include	<Stocks.h>

//****************************************************************************//
//      Stock constants                                                       //
//****************************************************************************//

//============================================================================//
//      Stock list elements                                                   //
//============================================================================//
# define	STOCK_DATA_TAG		"data"
# define	STOCK_STOCK_TAG		"stock"
# define	STOCK_TZONE_TAG		"timezone"

//============================================================================//
//      Stock list attributes                                                 //
//============================================================================//
# define	STOCK_TICKER_ATTR	"ticker"
# define	STOCK_NAME_ATTR		"name"
# define	STOCK_COUNTRY_ATTR	"country"
# define	STOCK_SECTOR_ATTR	"sector"
# define	STOCK_INDUSTRY_ATTR	"industry"
# define	STOCK_URL_ATTR		"url"
# define	STOCK_FILE_ATTR		"file"

//****************************************************************************//
//      Stock list structure                                                  //
//****************************************************************************//
struct StockList
{
	GtkListStore	*list;		// Stock list
	gchar			*tzone;		// Stock time zone
};

//****************************************************************************//
//      Internal functions                                                    //
//****************************************************************************//

//============================================================================//
//      Parse stock list line                                                 //
//============================================================================//
static void ParseLine (const gchar *string, gpointer data, GError **error)
{
	// Split string into tokens
	gchar **tokens = g_strsplit_set (string, "\t", 0);

	// Get count of tokens
	guint size = g_strv_length (tokens);
	if (size)
	{
		// Set stock fields to default values
		gchar *ticker = NULL;
		gchar *name = NULL;
		gchar *country = NULL;
		gchar *sector = NULL;
		gchar *industry = NULL;
		gchar *url = NULL;

		// Init stock fields
		if (size > 0)
			ticker = tokens[0];
		if (size > 1)
			name = tokens[1];
		if (size > 2)
			country = tokens[2];
		if (size > 3)
			sector = tokens[3];
		if (size > 4)
			industry = tokens[4];
		if (size > 5)
			url = tokens[5];

		// Strip stock fields
		ticker = g_strstrip (ticker);
		name = g_strstrip (name);
		country = g_strstrip (country);
		sector = g_strstrip (sector);
		industry = g_strstrip (industry);
		url = g_strstrip (url);

		// Check if ticker value is not empty
		if (!CheckField (ticker, "ticker", error))
		{
			g_strfreev (tokens);
			return;
		}

		// Check if name value is not empty
		if (!CheckField (name, "name", error))
		{
			g_strfreev (tokens);
			return;
		}

		// Convert data pointer
		GtkListStore *list = reinterpret_cast <GtkListStore*> (data);

		// Convert ticker to upper case
		gchar *upper = g_utf8_strup (ticker, -1);

		// Add new element to list store object
		GtkTreeIter iter;
		gtk_list_store_append (GTK_LIST_STORE (list), &iter);
		gtk_list_store_set (GTK_LIST_STORE (list), &iter, STOCK_TICKER_ID, upper, STOCK_NAME_ID, name, STOCK_COUNTRY_ID, country, STOCK_SECTOR_ID, sector, STOCK_INDUSTRY_ID, industry, STOCK_URL_ID, url, STOCK_CHECK_ID, FALSE, -1);

		// Free temporary string buffer
		g_free (upper);
	}

	// Release array of strings
	g_strfreev (tokens);
}

//============================================================================//
//      Parse stock list element                                              //
//============================================================================//
static void ParseElement (GMarkupParseContext *context, const gchar *element_name, const gchar **attribute_names, const gchar **attribute_values, gpointer data, GError **error)
{
	// Check element name and go to appropriate code branch
	if (g_utf8_collate (element_name, STOCK_STOCK_TAG) == 0)
	{
		// Local variables
		gchar *ticker, *name, *country, *sector, *industry, *url;

		// Collect stock attributes
		if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error, G_MARKUP_COLLECT_STRING, STOCK_TICKER_ATTR, &ticker, G_MARKUP_COLLECT_STRING, STOCK_NAME_ATTR, &name, G_MARKUP_COLLECT_STRING, STOCK_COUNTRY_ATTR, &country, G_MARKUP_COLLECT_STRING, STOCK_SECTOR_ATTR, &sector, G_MARKUP_COLLECT_STRING, STOCK_INDUSTRY_ATTR, &industry, G_MARKUP_COLLECT_STRING, STOCK_URL_ATTR, &url, G_MARKUP_COLLECT_INVALID))
		{
			// Strip stock fields
			ticker = g_strstrip (ticker);
			name = g_strstrip (name);
			country = g_strstrip (country);
			sector = g_strstrip (sector);
			industry = g_strstrip (industry);
			url = g_strstrip (url);

			// Check if ticker value is not empty
			if (!CheckField (ticker, "ticker", error))
				return;

			// Check if name value is not empty
			if (!CheckField (name, "name", error))
				return;

			// Convert data pointer
			StockList *slist = reinterpret_cast <StockList*> (data);

			// Convert ticker to upper case
			gchar *upper = g_utf8_strup (ticker, -1);

			// Add new element to list store object
			GtkTreeIter iter;
			gtk_list_store_append (GTK_LIST_STORE (slist -> list), &iter);
			gtk_list_store_set (GTK_LIST_STORE (slist -> list), &iter, STOCK_TICKER_ID, upper, STOCK_NAME_ID, name, STOCK_COUNTRY_ID, country, STOCK_SECTOR_ID, sector, STOCK_INDUSTRY_ID, industry, STOCK_URL_ID, url, STOCK_CHECK_ID, FALSE, -1);

			// Free temporary string buffer
			g_free (upper);
		}
	}
	else if (g_utf8_collate (element_name, STOCK_TZONE_TAG) == 0)
	{
		// Local variables
		const gchar *tzone;

		// Collect timezone attributes
		if (g_markup_collect_attributes (element_name, attribute_names, attribute_values, error, G_MARKUP_COLLECT_STRING, STOCK_FILE_ATTR, &tzone, G_MARKUP_COLLECT_INVALID))
		{
			// Check if timezone attribute is not empty
			if (!CheckField (tzone, STOCK_TZONE_TAG, error))
				return;

			// Convert data pointer
			StockList *slist = reinterpret_cast <StockList*> (data);

			// Free temporary string buffer
			g_free (slist -> tzone);

			// Set new time zone file name
			slist -> tzone = g_strdup (tzone);
		}
	}
	else if (g_utf8_collate (element_name, STOCK_DATA_TAG) == 0)
	{
		// Skip element processing
		return;
	}
	else
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Unknown element '%s'", element_name);
	}
}

//============================================================================//
//      Check stocks for duplicates                                           //
//============================================================================//
static gboolean CheckDuplicates (GtkTreeSortable *list, GError **error)
{
	// Operation status
	gboolean status = TRUE;

	// Sort stocks by ticker
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list), STOCK_TICKER_ID, GTK_SORT_ASCENDING);

	// Get iterator position
	GtkTreeIter iter;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
	{
		// Get stock ticker
		gchar *curr;
		gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, STOCK_TICKER_ID, &curr, -1);

		// Change iterator position to next element
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter))
		{
			// Set previous ticker value
			gchar *prev = curr;

			// Get stock ticker
			gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, STOCK_TICKER_ID, &curr, -1);

			// Check if previous and current tickers are equal
			if (g_utf8_collate (prev, curr) == 0)
			{
				// Set error message
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Found duplicate stock ticker '%s'", curr);

				// Free temporary string buffers
				g_free (prev);
				g_free (curr);

				// Return fail status
				return FALSE;
			}

			// Free temporary string buffer
			g_free (prev);
		}

		// Free temporary string buffer
		g_free (curr);
	}

	// Return operation status
	return status;
}

//****************************************************************************//
//      Constructor                                                           //
//****************************************************************************//
Stocks::Stocks (void)
{
	// Set stock elements to default values
	list = NULL;
	timezone = NULL;
}

//****************************************************************************//
//      Destructor                                                            //
//****************************************************************************//
Stocks::~Stocks (void)
{
	// Clear stock list
	this -> ClearList ();
}

//****************************************************************************//
//      Create new stock list                                                 //
//****************************************************************************//
void Stocks::NewList (const gchar *tzone)
{
	// Free stock elements
	if (list)
	{
		// Clear stock list
		gtk_list_store_clear (list);

		// Decrement reference count to stock list
		g_object_unref (list);
	}

	// Free string buffer
	g_free (timezone);

	// Set new stock elements
	list = gtk_list_store_new (STOCK_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	timezone = g_strdup (tzone);

	// Sort stocks by ticker
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list), STOCK_TICKER_ID, GTK_SORT_ASCENDING);
}

//****************************************************************************//
//      Close stock list                                                      //
//****************************************************************************//
void Stocks::ClearList (void)
{
	// Free stock elements
	if (list)
	{
		// Clear stock list
		gtk_list_store_clear (list);

		// Decrement reference count to stock list
		g_object_unref (list);
	}

	// Free string buffer
	g_free (timezone);

	// Set stock elements to default values
	list = NULL;
	timezone = NULL;
}

//****************************************************************************//
//      Open stock list from file                                             //
//****************************************************************************//
gboolean Stocks::OpenList (const gchar *fname, GError **error)
{
	// Try to load file content into string buffer
	gchar *content;
	gsize bytes;
	gboolean status = g_file_get_contents (fname, &content, &bytes, error);
	if (status)
	{
		// Create new stock list
		GtkListStore *newlist = gtk_list_store_new (STOCK_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

		// Create new stock list structure
		StockList slist = {newlist, NULL};

		// Create XML parser
		GMarkupParser parser = {ParseElement, NULL, NULL, NULL, NULL};

		// Create new XML parser context
		GMarkupParseContext *context = g_markup_parse_context_new (&parser, G_MARKUP_PREFIX_ERROR_POSITION, &slist, NULL);

		// Try to parse XML document
		if (!g_markup_parse_context_parse (context, content, bytes, error) || !g_markup_parse_context_end_parse (context, error))
		{
			// Release XML parser context
			g_markup_parse_context_free (context);

			// Free temporary string buffers
			g_free (content);
			g_free (slist.tzone);

			// Clear new stock list
			gtk_list_store_clear (newlist);

			// Decrement reference count to new stock list
			g_object_unref (newlist);

			// Return fail status
			return FALSE;
		}

		// Release XML parser context
		g_markup_parse_context_free (context);

		// Free temporary string buffer
		g_free (content);

		// Check if time zone file name is not empty
		if (slist.tzone == NULL)
		{
			// Set error message
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Missing stock list time zone.");
		}
		else
		{
			// Check stocks for duplicates
			if (CheckDuplicates (GTK_TREE_SORTABLE (newlist), error))
			{
				// Free stock elements
				if (list)
				{
					// Clear stock list
					gtk_list_store_clear (list);

					// Decrement reference count to stock list
					g_object_unref (list);
				}

				// Free string buffer
				g_free (timezone);

				// Set new stock elements
				list = newlist;
				timezone = slist.tzone;

				// Return success state
				return TRUE;
			}
		}

		// Free temporary string buffer
		g_free (slist.tzone);

		// Clear new stock list
		gtk_list_store_clear (newlist);

		// Decrement reference count to new stock list
		g_object_unref (newlist);

		// Return fail status
		return FALSE;
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Save stock list into file                                             //
//****************************************************************************//
gboolean Stocks::SaveList (const gchar *fname, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Check if stock list is set
	if (list)
	{
		// Start data section of XML file
		g_string_append_printf (string, "<" STOCK_DATA_TAG ">\n");

		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
		{
			// Iterate through all elements
			do {
				// Get stock details
				gchar *ticker, *name, *country, *sector, *industry, *url;
				gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, STOCK_TICKER_ID, &ticker, STOCK_NAME_ID, &name, STOCK_COUNTRY_ID, &country, STOCK_SECTOR_ID, &sector, STOCK_INDUSTRY_ID, &industry, STOCK_URL_ID, &url, -1);

				// Escapes text according to XML rules
				gchar *text = g_markup_printf_escaped ("<" STOCK_STOCK_TAG " " STOCK_TICKER_ATTR "=\"%s\" " STOCK_NAME_ATTR "=\"%s\" " STOCK_COUNTRY_ATTR "=\"%s\" " STOCK_SECTOR_ATTR "=\"%s\" " STOCK_INDUSTRY_ATTR "=\"%s\" " STOCK_URL_ATTR "=\"%s\"/>\n", ticker, name, country, sector, industry, url);

				// Append stock information into string buffer
				string = g_string_append (string, text);

				// Free temporary string buffers
				g_free (ticker);
				g_free (name);
				g_free (country);
				g_free (sector);
				g_free (industry);
				g_free (url);
				g_free (text);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));
		}

		// End data section of XML file
		g_string_append_printf (string, "</" STOCK_DATA_TAG ">\n");

		// Store time zone file name
		g_string_append_printf (string, "<" STOCK_TZONE_TAG " " STOCK_FILE_ATTR "=\"%s\"/>\n", timezone);
	}

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Import stock list from file                                           //
//****************************************************************************//
gboolean Stocks::ImportList (const gchar *fname, GError **error)
{
	// Try to load file content into string buffer
	gchar *content;
	gsize bytes;
	gboolean status = g_file_get_contents (fname, &content, &bytes, error);
	if (status)
	{
		// Create new stock list
		GtkListStore *newlist = gtk_list_store_new (STOCK_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

		// Split file content into lines
		gchar **lines = g_strsplit_set (content, "\n", 0);

		// Process all file lines
		gint line = 1;
		gchar **array = lines;
		while (*array)
		{
			// Parse file line
			ParseLine (*array, newlist, error);

			// Check if error is occured
			if (*error)
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line);

				// Release array of strings
				g_strfreev (lines);

				// Free temporary string buffer
				g_free (content);

				// Clear new stock list
				gtk_list_store_clear (newlist);

				// Decrement reference count to new stock list
				g_object_unref (newlist);

				// Return fail status
				return FALSE;
			}

			// Go to next file line
			line++;
			array++;
		}

		// Release array of strings
		g_strfreev (lines);

		// Free temporary string buffer
		g_free (content);

		// Check stocks for duplicates
		if (CheckDuplicates (GTK_TREE_SORTABLE (newlist), error))
		{
			// Free stock elements
			if (list)
			{
				// Clear stock list
				gtk_list_store_clear (list);

				// Decrement reference count to stock list
				g_object_unref (list);
			}

			// Set new stock elements
			list = newlist;

			// Return success state
			return TRUE;
		}

		// Clear new stock list
		gtk_list_store_clear (newlist);

		// Decrement reference count to new stock list
		g_object_unref (newlist);

		// Return fail status
		return FALSE;
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Export stock list into file                                           //
//****************************************************************************//
gboolean Stocks::ExportList (const gchar *fname, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Check if stock list is set
	if (list)
	{
		// Get iterator position
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
		{
			// Iterate through all elements
			do {
				// Get stock details
				gchar *ticker, *name, *country, *sector, *industry, *url;
				gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, STOCK_TICKER_ID, &ticker, STOCK_NAME_ID, &name, STOCK_COUNTRY_ID, &country, STOCK_SECTOR_ID, &sector, STOCK_INDUSTRY_ID, &industry, STOCK_URL_ID, &url, -1);

				// Append stock information into string buffer
				g_string_append_printf (string, "%s\t%s\t%s\t%s\t%s\t%s\n", ticker, name, country, sector, industry, url);

				// Free temporary string buffers
				g_free (ticker);
				g_free (name);
				g_free (country);
				g_free (sector);
				g_free (industry);
				g_free (url);

				// Change iterator position to next element
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));
		}
	}

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Get stock list                                                        //
//****************************************************************************//
GtkListStore* Stocks::GetStockList (void) const
{
	return list;
}

//****************************************************************************//
//      Get stock time zone property                                          //
//****************************************************************************//
gchar* Stocks::GetTimeZone (void) const
{
	return g_strdup (timezone);
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

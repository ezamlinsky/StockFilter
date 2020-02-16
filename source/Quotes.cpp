/*                                                                    Quotes.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 QUOTES CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<QuoteList.h>
# include	<Quotes.h>
# include	<Math.h>
# include	<Array.h>
# include	<Statistics.h>

//****************************************************************************//
//      Internal functions                                                    //
//****************************************************************************//

//============================================================================//
//      Quote compare function                                                //
//============================================================================//
static sint64_t QuoteCompare (const void *key1, const void *key2)
{
	// Convert key pointers
	const quote_t *ptr1 = reinterpret_cast <const quote_t*> (key1);
	const quote_t *ptr2 = reinterpret_cast <const quote_t*> (key2);

	// Compare quote dates
	return Math::Compare (ptr1 -> date, ptr2 -> date);
}

//============================================================================//
//      Check quote list for errors                                           //
//============================================================================//
static gsize CheckQuoteslist (const quote_t *list[], gsize size, GError **error)
{
	// Set target and source pointers
	const quote_t **target = list;
	const quote_t **source = list;

	// Set previous time stamp
	time_t prev = TIME_ERROR - 1;

	// Check all quotes
	while (size)
	{
		// Check quote time stamp
		if (source[0] -> date >= prev)
		{
			// Set error message
			date_struct curdate = Time::ExtractDate (source[0] -> date);
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Found duplicate timestamp %.4i-%.2d-%.2d", curdate.year, curdate.mon, curdate.day);

			// Return fail status
			return -1;
		}

		// Check if quote correct
		if (!IsQuoteCorrect (source[0] -> date, source[0] -> open, source[0] -> high, source[0] -> low, source[0] -> close, error))
		{
			// Return fail status
			return -1;
		}

		// Remove indicative quotes for week ends
		if (static_cast <uint8_t> (Time::WeekDay (source[0] -> date) - 1) < 5)
		{
			target[0] = source[0];
			target++;
		}

		// Set previous time stamp
		prev = source[0] -> date;

		// Go to next stock quote
		source++;
		size--;
	}

	// Return corrected list size
	return target - list;
}

//****************************************************************************//
//      Global functions                                                      //
//****************************************************************************//

//============================================================================//
//      Extract quotes from string buffer                                     //
//============================================================================//
gsize ExtractQuotes (const gchar *buffer, Accumulator *accumulator, GError **error)
{
	// Init records count
	gsize count = 0;

	// Split buffer into lines
	gchar **lines = g_strsplit_set (buffer, "\n", 0);

	// Process all file lines
	gchar **lptr = lines;
	gint line = 0;
	while (*lptr)
	{
		// Split string into tokens
		gchar **tokens = g_strsplit_set (*lptr, ",\t", 0);

		// Get count of tokens
		guint size = g_strv_length (tokens);
		if (size)
		{
			// Set quote fields to default values
			const gchar *date = "";
			const gchar *open = "";
			const gchar *high = "";
			const gchar *low = "";
			const gchar *close = "";
			const gchar *adjclose = "";
			const gchar *volume = "";

			// Init quote fields
			if (size > 0)
				date = tokens[0];
			if (size > 1)
				open = tokens[1];
			if (size > 2)
				high = tokens[2];
			if (size > 3)
				low = tokens[3];
			if (size > 4)
				close = tokens[4];
			if (size > 5)
				volume = tokens[5];
			if (size > 6)
				adjclose = tokens[6];

			// Extract date from string
			time_t qdate = ExtractDate (date, error);

			// Check if date is correct
			if (qdate == static_cast <time_t> (TIME_ERROR))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract open price from string
			gfloat qopen = ExtractPrice (open, "open", error);

			// Check if open price is correct
			if (Math::IsNaN (qopen))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract high price from string
			gfloat qhigh = ExtractPrice (high, "high", error);

			// Check if high price is correct
			if (Math::IsNaN (qhigh))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract low price from string
			gfloat qlow = ExtractPrice (low, "low", error);

			// Check if low price is correct
			if (Math::IsNaN (qlow))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract close price from string
			gfloat qclose = ExtractPrice (close, "close", error);

			// Check if close price is correct
			if (Math::IsNaN (qclose))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract adjusted close price from string
			gfloat qadjclose = ExtractPrice (adjclose, "adjusted close", error);

			// Check if adjclose price is correct
			if (Math::IsNaN (qadjclose))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Extract volume from string
			gsize qvolume = ExtractVolume (volume, error);

			// Check if volume is correct
			if (qvolume == static_cast <gsize> (-1))
			{
				// Set error message prefix
				g_prefix_error (error, "Line %i: ", line + 2);

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Reserve space into accumulator
			quote_t *pos = reinterpret_cast <quote_t*> (accumulator -> Reserve (sizeof (quote_t)));
			if (pos == NULL)
			{
				// Set error message
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Can not reserve more space for quotes buffer");

				// Release array of tokens
				g_strfreev (tokens);

				// Release array of strings
				g_strfreev (lines);

				// Return error state
				return -1;
			}

			// Copy data into accumulator
			pos[0].date = qdate;
			pos[0].open = qopen;
			pos[0].high = qhigh;
			pos[0].low = qlow;
			pos[0].close = qclose;
			pos[0].adjclose = qadjclose;
			pos[0].volume = qvolume;

			// Mark allocated accumulator space as filled by data
			accumulator -> Fill (sizeof (quote_t));

			// Increment records count
			count++;
		}

		// Release array of tokens
		g_strfreev (tokens);

		// Go to next file line
		line++;
		lptr++;
	}

	// Release array of strings
	g_strfreev (lines);

	// Return quotes count
	return count;
}

//============================================================================//
//      Check quotes array for errors                                         //
//============================================================================//
QuoteList CheckQuotes (const quote_t *array, gsize size, GError **error)
{
	// Init result structure
	QuoteList result = {NULL, static_cast <gsize> (-1)};

	// Create array of pointers
	const quote_t *ptr[size];

	// Fill array of pointers
	const quote_t *source = array;
	const quote_t **target = ptr;
	gsize count = size;
	while (count)
	{
		target[0] = source;
		source++;
		target++;
		count--;
	}

	// Sort quotes by date
	Array::QuickSortDsc (reinterpret_cast <const void**> (ptr), size, QuoteCompare);

	// Check quotes list for errors
	size = CheckQuoteslist (ptr, size, error);
	if (size != static_cast <gsize> (-1))
	{
		// Allocate memory for quotes array
		quote_t *quotes = reinterpret_cast <quote_t*> (g_malloc (size * sizeof (quote_t)));

		// Copy sorted quotes into quotes array
		const quote_t **sptr = ptr;
		quote_t *tptr = quotes;
		count = size;
		while (count)
		{
			tptr[0] = *sptr[0];
			sptr++;
			tptr++;
			count--;
		}

		// Set result structure fields
		result.array = quotes;
		result.size = size;
	}

	// Normal exit
	return result;
}

//****************************************************************************//
//      Constructor                                                           //
//****************************************************************************//
Quotes::Quotes (void)
{
	// Set quote elements to default values
	array = NULL;
	size = 0;
	synctime = TIME_ERROR;
}

//****************************************************************************//
//      Destructor                                                            //
//****************************************************************************//
Quotes::~Quotes (void)
{
	// Free quote elements
	g_free (array);

	// Set quote elements to default values
	array = NULL;
	size = 0;
	synctime = TIME_ERROR;
}

//****************************************************************************//
//      Create new quote list                                                 //
//****************************************************************************//
void Quotes::NewList (time_t stime)
{
	// Free quote elements
	g_free (array);

	// Set new quote elements
	array = NULL;
	size = 0;
	synctime = stime;
}

//****************************************************************************//
//      Open quote list from file                                             //
//****************************************************************************//
gboolean Quotes::OpenList (const gchar *fname, GError **error)
{
	// Try to load file content into string buffer
	gchar *content;
	gsize bytes;
	gboolean status = g_file_get_contents (fname, &content, &bytes, error);
	if (status)
	{
		// Check file size
		bytes -= sizeof (time_t);
		if (bytes % sizeof (quote_t))
		{
			// Set error message
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Quotes file is corrupted");

			// Set fail status
			status = FALSE;
		}
		else
		{
			// Check stock quotes for errors
			bytes /= sizeof (quote_t);
			QuoteList result = CheckQuotes (reinterpret_cast <const quote_t*> (content), bytes, error);
			if (result.size == static_cast <gsize> (-1))
			{
				// Set fail status
				status = FALSE;
			}
			else
			{
				// Free quote elements
				g_free (array);

				// Set new quote elements
				array = const_cast <quote_t*> (result.array);
				size = result.size;
				synctime = *reinterpret_cast <time_t*> (reinterpret_cast <quote_t*> (content) + bytes);
			}
		}

		// Free temporary string buffer
		g_free (content);
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Save quote list into file                                             //
//****************************************************************************//
gboolean Quotes::SaveList (const gchar *fname, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Store quotes array into string buffer
	string = g_string_append_len (string, reinterpret_cast <const gchar*> (array), size * sizeof (quote_t));

	// Store sync time
	string = g_string_append_len (string, reinterpret_cast <const gchar*> (&synctime), sizeof (time_t));

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Import quote list from file                                           //
//****************************************************************************//
gboolean Quotes::ImportList (const gchar *fname, GError **error)
{
	// Try to load file content into string buffer
	gchar *content;
	gsize bytes;
	gboolean status = g_file_get_contents (fname, &content, &bytes, error);
	if (status)
	{
		// Skip quotes header
		const gchar *pos = g_utf8_strchr (content, -1, '\n');
		if (pos == NULL)
		{
			// Set error message
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Quote list is empty");

			// Set fail status
			status = FALSE;
		}
		else
		{
			// Go to first quote row
			pos += sizeof (gchar);

			// Create accumulator object
			Accumulator accumulator (0);

			// Extract quotes from string buffer
			gsize count = ExtractQuotes (pos, &accumulator, error);
			if (count == static_cast <gsize> (-1))
			{
				// Set fail status
				status = FALSE;
			}
			else
			{
				// Check stock quotes for errors
				QuoteList result = CheckQuotes (reinterpret_cast <const quote_t*> (accumulator.Data ()), count, error);
				if (result.size == static_cast <gsize> (-1))
				{
					// Set fail status
					status = FALSE;
				}
				else
				{
					// Free quote elements
					g_free (array);

					// Set new quote elements
					array = const_cast <quote_t*> (result.array);
					size = result.size;
				}
			}
		}

		// Free temporary string buffer
		g_free (content);
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Export quote list into file                                           //
//****************************************************************************//
gboolean Quotes::ExportList (const gchar *fname, GError **error)
{
	// Create string buffer
	GString *string = g_string_new (NULL);

	// Append quotes header
	g_string_append_printf (string, "Date\tOpen\tHigh\tLow\tClose\tVolume\tAdjusted close\n");

	// Fill string buffer
	quote_t *ptr = array;
	gsize count = size;
	while (count)
	{
		// Append quote information into string buffer
		date_struct curdate = Time::ExtractDate (ptr[0].date);
		g_string_append_printf (string, "%.4i-%.2d-%.2d\t%.2f\t%.2f\t%.2f\t%.2f\t%ld\t%.2f\n", curdate.year, curdate.mon, curdate.day, ptr[0].open, ptr[0].high, ptr[0].low, ptr[0].close, ptr[0].volume, ptr[0].adjclose);

		// Go to next stock quote
		ptr++;
		count--;
	}

	// Try to save string buffer into file
	gboolean status = g_file_set_contents (fname, string -> str, string -> len, error);

	// Relase string buffer
	g_string_free (string, TRUE);

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Add new quotes to quote list                                          //
//****************************************************************************//
gboolean Quotes::AddQuotes (QuoteList newlist, time_t stime, GError **error)
{
	// Check if new list is not empty
	if (newlist.size == 0)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "No new quotes");

		// Return fail status
		return FALSE;
	}

	// Get first quote date of new quote list
	time_t first = newlist.array[newlist.size-1].date;

	// Get last quote date of old quote list
	time_t last = TIME_ERROR;
	if (size)
		last = array[0].date;

	// Check if quotes do not overlap
	if (first <= last)
	{
		// Set error message
		date_struct fdate = Time::ExtractDate (first);
		date_struct ldate = Time::ExtractDate (last);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "First quote date %.4i-%.2d-%.2d overlaps last quote date %.4i-%.2d-%.2d", fdate.year, fdate.mon, fdate.day, ldate.year, ldate.mon, ldate.day);

		// Return fail status
		return FALSE;
	}
	else
	{
		// Allocate memory for quotes array
		quote_t *quotes = reinterpret_cast <quote_t*> (g_malloc ((size + newlist.size) * sizeof (quote_t)));

		// Copy quotes into new quotes array
		Array::Copy (quotes, newlist.array, newlist.size * sizeof (quote_t));
		Array::Copy (quotes + newlist.size, array, size * sizeof (quote_t));

		// Free quote elements
		g_free (array);

		// Set new quote elements
		array = quotes;
		size = size + newlist.size;
		synctime = stime;

		// Return success status
		return TRUE;
	}
}

//****************************************************************************//
//      Set quote list                                                        //
//****************************************************************************//
gboolean Quotes::SetQuotes (GtkListStore *list, GError **error)
{
	// Operation status
	gboolean status = FALSE;

	// Get quote list size
	gsize count = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list), NULL);

	// Create temporary quotes array
	quote_t temp[count];

	// Get iterator position
	GtkTreeIter iter;
	quote_t *ptr = temp;
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
	{
		// Iterate through all elements
		do {
			// Set stock quote fields
			gtk_tree_model_get (GTK_TREE_MODEL (list), &iter, QUOTE_DATE_ID, &ptr[0].date, QUOTE_OPEN_ID, &ptr[0].open, QUOTE_HIGH_ID, &ptr[0].high, QUOTE_LOW_ID, &ptr[0].low, QUOTE_CLOSE_ID, &ptr[0].close, QUOTE_ADJCLOSE_ID, &ptr[0].adjclose, QUOTE_VOLUME_ID, &ptr[0].volume, -1);

			// Go to next stock quote
			ptr++;

			// Change iterator position to next element
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));
	}

	// Check stock quotes for errors
	QuoteList result = CheckQuotes (temp, count, error);
	if (result.size != static_cast <gsize> (-1))
	{
		// Free quote elements
		g_free (array);

		// Set new quote elements
		array = const_cast <quote_t*> (result.array);
		size = result.size;

		// Set success status
		status = TRUE;
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Get quote list                                                        //
//****************************************************************************//
GtkListStore* Quotes::GetQuotes (void) const
{
	// Create quote list
	GtkListStore *list = gtk_list_store_new (QUOTE_COLUMNS, G_TYPE_INT64, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_UINT64);

	// Fill quote list
	quote_t *ptr = array;
	gsize count = size;
	while (count)
	{
		// Add new element to list store object
		GtkTreeIter iter;
		gtk_list_store_append (GTK_LIST_STORE (list), &iter);
		gtk_list_store_set (GTK_LIST_STORE (list), &iter, QUOTE_DATE_ID, ptr[0].date, QUOTE_OPEN_ID, ptr[0].open, QUOTE_HIGH_ID, ptr[0].high, QUOTE_LOW_ID, ptr[0].low, QUOTE_CLOSE_ID, ptr[0].close, QUOTE_ADJCLOSE_ID, ptr[0].adjclose, QUOTE_VOLUME_ID, ptr[0].volume, -1);

		// Go to next stock quote
		ptr++;
		count--;
	}

	// Sort quotes by date
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list), QUOTE_DATE_ID, GTK_SORT_DESCENDING);

	// Return quote list
	return list;
}

//****************************************************************************//
//      Get quote list                                                        //
//****************************************************************************//
QuoteList Quotes::GetQuoteList (void) const
{
	return {array, size};
}

//****************************************************************************//
//      Get quotes count                                                      //
//****************************************************************************//
gint Quotes::GetCount (void) const
{
	return size;
}

//****************************************************************************//
//      Get quotes sync time                                                  //
//****************************************************************************//
time_t Quotes::GetSyncTime (void) const
{
	return synctime;
}

//****************************************************************************//
//      Get first quote date                                                  //
//****************************************************************************//
time_t Quotes::GetFirstDate (void) const
{
	// Check if quotes array has stock quotes
	if (size)
	{
		// Return first quote date
		return array[size-1].date;
	}
	else
	{
		// In case of error return TIME_ERROR
		return TIME_ERROR;
	}
}

//****************************************************************************//
//      Get last quote date                                                   //
//****************************************************************************//
time_t Quotes::GetLastDate (void) const
{
	// Check if quotes array has stock quotes
	if (size)
	{
		// Return last quote date
		return array[0].date;
	}
	else
	{
		// In case of error return TIME_ERROR
		return TIME_ERROR;
	}
}

//****************************************************************************//
//      Get last quote price                                                  //
//****************************************************************************//
gfloat Quotes::GetLastPrice (void) const
{
	// Check if quotes array has stock quotes
	if (size)
	{
		// Return last close price
		return array[0].close;
	}
	else
	{
		// In case of error return 0
		return 0;
	}
}

//****************************************************************************//
//      Get stock liquidity                                                   //
//****************************************************************************//
gsize Quotes::GetLiquidity (gsize count) const
{
	// Correct count value
	if (count > size)
		count = size;

	// Create liquidity array
	gsize liquidity[count];

	// Fill liquidity array
	quote_t *source = array;
	gsize *target = liquidity;
	gsize hsize = count;
	while (count)
	{
		target[0] = source[0].volume;
		source++;
		target++;
		count--;
	}

	// Return stock liquidity
	return Statistics::Median (liquidity, hsize);
}

//****************************************************************************//
//      Get stock volatility                                                  //
//****************************************************************************//
gfloat Quotes::GetVolatility (gsize count) const
{
	// Correct count value
	if (count > size)
		count = size;

	// Create volatility array
	gfloat volatility [count];

	// Fill volatility array
	quote_t *source = array;
	gfloat *target = volatility;
	gsize hsize = count;
	while (count)
	{
		target[0] = Math::Log (source[0].high / source[0].low);
		source++;
		target++;
		count--;
	}

	// Return stock volatility
	return Statistics::Median (volatility, hsize);
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/
// TODO: Удалить
/*
gfloat InitArray (gfloat target[], quote_t source[], size_t size)
{
	gfloat mean = 0;
	gsize count = size;
	while (count)
	{
		gfloat value = Math::Log (source[0].high / source[0].low);
		mean += value;
		target[0] = value;
		source++;
		target++;
		count--;
	}

	return mean / size;
}
gfloat InitArray1 (gfloat target[], quote_t source[], size_t size)
{
	gfloat mean = 0;
	gsize count = size;
	while (count)
	{
		gfloat value = Math::Abs(Math::Log (source[0].open / source[1].close));
		mean += value;
		target[0] = value;
		source++;
		target++;
		count--;
	}

	return mean / size;
}
gfloat Correlation (const gfloat array1[], const gfloat array2[], size_t size, gfloat mean1, gfloat mean2)
{
	gfloat sum0 = 0;
	gfloat sum1 = 0;
	gfloat sum2 = 0;
	while (size)
	{
		gfloat temp0 = array1[0] - mean1;
		gfloat temp1 = array2[0] - mean2;
		sum0 += temp0 * temp1;
		sum1 += temp0 * temp0;
		sum2 += temp1 * temp1;
		array1++;
		array2++;
		size--;
	}
	return sum0 / Math::Sqrt (sum1 * sum2);
}
gfloat Quotes::GetVolatility (gsize count) const
{
	// Correct count value
	if (count > size)
		count = size;

	// Create volatility array
	gfloat arr1 [count - 1];
	gfloat arr2 [count - 1];

	gfloat mean1 = InitArray (arr1, array, count - 1);
	gfloat mean2 = InitArray1 (arr2, array, count - 1);

	return Correlation (arr1, arr2, count - 1, mean1, mean2);
}
*/

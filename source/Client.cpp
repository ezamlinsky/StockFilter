/*                                                                    Client.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                                 CLIENT CLASS                                 #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<QuoteList.h>
# include	<Client.h>
# include	<Array.h>

//****************************************************************************//
//      Internal constants                                                    //
//****************************************************************************//
# define	BUFFER_SIZE		4096	// Static buffer size
# define	KEEPIDLE		30		// TCP keep-alive idle time wait
# define	KEEPINTVL		5		// TCP keep-alive interval
# define	MAXCONNECTS		5		// Maximum connection cache size

//****************************************************************************//
//      Callback function data structure                                      //
//****************************************************************************//
struct FuncData
{
	Accumulator	*accumulator;		// Accumulating buffer
	GError		**error;			// Pointer to position of Error object
};

//****************************************************************************//
//      Internal functions                                                    //
//****************************************************************************//

//============================================================================//
//      Curl callback function for collecting received data                   //
//============================================================================//
static gsize DataAccumulator (gchar *ptr, gsize size, gsize nmemb, gpointer data)
{
	// Convert data pointer
	FuncData *dptr = reinterpret_cast <FuncData*> (data);

	// Get amount of received bytes
	gsize bytes = size * nmemb;

	// Reserve space into accumulator
	void *pos = dptr -> accumulator -> Reserve (bytes);
	if (pos == NULL)
	{
		// Set error message
		g_set_error (dptr -> error, G_FILE_ERROR, G_FILE_ERROR_IO, "Can not reserve more space for sync buffer");

		// Return error status
		return -1;
	}

	// Copy data into accumulator
	Array::Copy (pos, ptr, bytes);

	// Mark allocated accumulator space as filled by data
	dptr -> accumulator -> Fill (bytes);

	// Normal exit
	return bytes;
}

//============================================================================//
//      Accumulate splits                                                     //
//============================================================================//
static gboolean AccumulateSplits (CURL *handle, Accumulator *accumulator, const gchar *ticker, time_t start, time_t end, GError **error)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Check handle
	if (handle == NULL)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Curl handle was not initialised correctly");

		// Return fail status
		return FALSE;
	}

	// Create callback function data
	struct FuncData data = {accumulator, error};

	// Compose server request
	date_struct sdate = Time::ExtractDate (start);
	date_struct edate = Time::ExtractDate (end);
	g_snprintf (buffer, BUFFER_SIZE, "http://real-chart.finance.yahoo.com/x?s=%s&a=%d&b=%d&c=%i&d=%d&e=%d&f=%i&g=v", ticker, sdate.mon - 1, sdate.day, sdate.year, edate.mon - 1, edate.day, edate.year);

	// Set data for curl callback function
	CURLcode result = curl_easy_setopt (handle, CURLOPT_WRITEDATA, &data);
	if (result == CURLE_OK)
	{
		// Set URL
		result = curl_easy_setopt (handle, CURLOPT_URL, buffer);
		if (result == CURLE_OK)
		{
			// Get qoutes from quote server
			result = curl_easy_perform (handle);
			if (result == CURLE_OK)
			{
				// Reserve space into accumulator
				gchar *pos = reinterpret_cast <gchar*> (accumulator -> Reserve (sizeof (gchar)));
				if (pos == NULL)
				{
					// Set error message
					g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Can not reserve more space for sync buffer");

					// Return fail status
					return FALSE;
				}

				// Set end of string marker
				*pos = '\0';

				// Mark allocated accumulator space as filled by data
				accumulator -> Fill (sizeof (gchar));

				// Return success state
				return TRUE;
			}
		}
	}

	// Get error desription
	const char *message = curl_easy_strerror (result);

	// Set error message
	g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, message);

	// Return fail status
	return FALSE;
}

//============================================================================//
//      Accumulate quotes                                                     //
//============================================================================//
static gboolean AccumulateQuotes (CURL *handle, Accumulator *accumulator, const gchar *ticker, time_t start, time_t end, GError **error)
{
	// Allocate space for static buffer
	gchar buffer [BUFFER_SIZE];

	// Check handle
	if (handle == NULL)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Curl handle was not initialised correctly");

		// Return fail status
		return FALSE;
	}

	// Create callback function data
	struct FuncData data = {accumulator, error};

	// Compose server request
	date_struct sdate = Time::ExtractDate (start);
	date_struct edate = Time::ExtractDate (end);
	g_snprintf (buffer, BUFFER_SIZE, "http://real-chart.finance.yahoo.com/table.csv?s=%s&a=%d&b=%d&c=%i&d=%d&e=%d&f=%i&g=d", ticker, sdate.mon - 1, sdate.day, sdate.year, edate.mon - 1, edate.day, edate.year);

	// Set data for curl callback function
	CURLcode result = curl_easy_setopt (handle, CURLOPT_WRITEDATA, &data);
	if (result == CURLE_OK)
	{
		// Set URL
		result = curl_easy_setopt (handle, CURLOPT_URL, buffer);
		if (result == CURLE_OK)
		{
			// Get qoutes from quote server
			result = curl_easy_perform (handle);
			if (result == CURLE_OK)
			{
				// Reserve space into accumulator
				gchar *pos = reinterpret_cast <gchar*> (accumulator -> Reserve (sizeof (gchar)));
				if (pos == NULL)
				{
					// Set error message
					g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Can not reserve more space for sync buffer");

					// Return fail status
					return FALSE;
				}

				// Set end of string marker
				*pos = '\0';

				// Mark allocated accumulator space as filled by data
				accumulator -> Fill (sizeof (gchar));

				// Return success state
				return TRUE;
			}
		}
	}

	// Get error desription
	const char *message = curl_easy_strerror (result);

	// Set error message
	g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, message);

	// Return fail status
	return FALSE;
}

//****************************************************************************//
//      Constructor                                                           //
//****************************************************************************//
Client::Client (void)
{
	// Set client elements to default values
	array = NULL;
	size = 0;
	handle = NULL;
}

//****************************************************************************//
//      Destructor                                                            //
//****************************************************************************//
Client::~Client (void)
{
	// Free client elements
	g_free (array);

	// Set client elements to default values
	array = NULL;
	size = 0;
	handle = NULL;
}

//****************************************************************************//
//      Client initialization                                                 //
//****************************************************************************//
gboolean Client::Init (GError **error)
{
	// Free client elements
	g_free (array);

	// Cleanup old handle
	curl_easy_cleanup (handle);

	// Set client elements to default values
	array = NULL;
	size = 0;

	// Get new handle
	handle = curl_easy_init ();
	if (handle == NULL)
	{
		// Set error message
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, "Error initializing curl handle");

		// Return fail status
		return FALSE;
	}

	// Set TCP keep-alive probing
	CURLcode result = curl_easy_setopt (handle, CURLOPT_TCP_KEEPALIVE, 1);
	if (result == CURLE_OK)
	{
		// Set TCP keep-alive idle time wait
		result = curl_easy_setopt (handle, CURLOPT_TCP_KEEPIDLE, KEEPIDLE);
		if (result == CURLE_OK)
		{
			// Set TCP keep-alive interval
			result = curl_easy_setopt (handle, CURLOPT_TCP_KEEPINTVL, KEEPINTVL);
			if (result == CURLE_OK)
			{
				// Set maximum connection cache size
				result = curl_easy_setopt (handle, CURLOPT_MAXCONNECTS, MAXCONNECTS);
				if (result == CURLE_OK)
				{
					// Set write callback function
					result = curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, DataAccumulator);
					if (result == CURLE_OK)
					{
						// Return success state
						return TRUE;
					}
				}
			}
		}
	}

	// Get error desription
	const char *message = curl_easy_strerror (result);

	// Set error message
	g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_IO, message);

	// Return fail status
	return FALSE;
}

//****************************************************************************//
//      Request quotes from quote server                                      //
//****************************************************************************//
gboolean Client::CheckSplits (const gchar *ticker, time_t start, time_t end, GError **error)
{
	// Try to get quotes from quote server
	Accumulator buffer (0);
	gboolean status = AccumulateSplits (handle, &buffer, ticker, start, end, error);
	if (status)
	{
		// Check for splits and dividends
		if (g_pattern_match_simple ("*DIVIDEND*", reinterpret_cast <const gchar*> (buffer.Data ())) || g_pattern_match_simple ("*SPLIT*", reinterpret_cast <const gchar*> (buffer.Data ())))
			return TRUE;
		else
			return FALSE;
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Request quotes from quote server                                      //
//****************************************************************************//
gboolean Client::GetQuotes (const gchar *ticker, time_t start, time_t end, GError **error)
{
	// Try to get quotes from quote server
	Accumulator buffer (0);
	gboolean status = AccumulateQuotes (handle, &buffer, ticker, start, end, error);
	if (status)
	{
		// Skip quotes header
		const gchar *pos = g_utf8_strchr (reinterpret_cast <const gchar*> (buffer.Data ()), -1, '\n');
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
	}

	// Return file operation status
	return status;
}

//****************************************************************************//
//      Get quote list                                                        //
//****************************************************************************//
QuoteList Client::GetQuoteList (void) const
{
	return {array, size};
}

//****************************************************************************//
//      Get quotes count                                                      //
//****************************************************************************//
gint Client::GetCount (void) const
{
	return size;
}

//****************************************************************************//
//      Get first quote date                                                  //
//****************************************************************************//
time_t Client::GetFirstDate (void) const
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
time_t Client::GetLastDate (void) const
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
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

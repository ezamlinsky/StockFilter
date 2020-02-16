/*                                                                  TimeZone.cpp
################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                               TIME ZONE CLASS                                #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################
*/
# include	<string.h>
# include	<TimeZone.h>

//****************************************************************************//
//      Load time zone from tzdata file                                       //
//****************************************************************************//
gboolean TimeZone::Init (const gchar *tzone, GError **error)
{
	// Load time zone data from tzdata file
	error_t errno = time.LoadTimeZone (tzone);
	if (errno)
	{
		// Get error description
		const gchar *message = strerror (errno);

		// Set error message
		g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno), "Error loading time zone file '%s': %s", tzone, message);

		// Return fail status
		return FALSE;
	}

	// Return success state
	return TRUE;
}

//****************************************************************************//
//      Get current time in time zone                                         //
//****************************************************************************//
time_t TimeZone::GetCurrentTime (void)
{
	// Return current local time into time zone
	return time.LocalTime (time.SystemTime ());
}
/*
################################################################################
#                                 END OF FILE                                  #
################################################################################
*/

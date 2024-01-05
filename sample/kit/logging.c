#include "ax/log2.h"

int main()
{
	ax_log_set_mode(AX_LM_NOLOC | AX_LM_NOTIME);

	ax_log(AX_LL_INFO, "Info message");
	ax_log(AX_LL_DEBUG, "Debug message");
	ax_log(AX_LL_WARN, "Warn message");
	ax_log(AX_LL_ERROR, "Error message");
	ax_log(AX_LL_FATAL, "Fatal message");
	ax_log(-42, "Fatal message with custom level -42");

	ax_log_set_mode(AX_LM_NOLOC);

	ax_pinfo("Info message with location");
	ax_pdebug("Debug message with location");
	ax_pwarn("Warn message with location");
	ax_perror("Error message with location");
	ax_pfatal("Fatal message with location");

	ax_log_set_mode(AX_LM_NOTIME);

	ax_pinfo("Info message with time");
	ax_pdebug("Debug message with time");
	ax_pwarn("Warn message with time");
	ax_perror("Error message with time");
	ax_pfatal("Fatal message with time");

	ax_log_set_mode(0);

	ax_pinfo("Info message with location and time");
	ax_pdebug("Debug message with location and time");
	ax_pwarn("Warn message with location and time");
	ax_perror("Error message with location and time");
	ax_pfatal("Fatal message with location and time");
}

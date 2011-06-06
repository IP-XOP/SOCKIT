#pragma rtGlobals=1		// Use modern global access method.
Function processorator(f, buffer, messages, [delimiter])
Funcref processFuncTemplate f
Wave/t buffer
string messages, delimiter

if(paramisdefault(delimiter))
	delimiter = "\n"
endif
variable ii, nummessages = itemsinlist(messages, delimiter)
for(ii = 0 ; ii < nummessages ; ii+=1)
	redimension/n=(dimsize(buffer, 0) + 1, -1) buffer
	buffer[dimsize(buffer, 0) - 1] = stringfromlist(ii, messages)
	f(buffer, ii)
endfor

End

Function processFuncTemplate(w, x)
wave/t w
variable x
End
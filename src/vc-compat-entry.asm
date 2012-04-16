.model flat
.data


EXTRN _xyzzySetDllDirectoryA@4:PROC
EXTERNDEF __imp__SetDllDirectoryA@4:DWORD
__imp__SetDllDirectoryA@4       dd _xyzzySetDllDirectoryA@4


ifdef VCVER
if VCVER ge 10

EXTRN _xyzzyEncodePointer@4:PROC
EXTRN _xyzzyDecodePointer@4:PROC

EXTERNDEF __imp__EncodePointer@4:DWORD
EXTERNDEF __imp__DecodePointer@4:DWORD

__imp__EncodePointer@4          dd _xyzzyEncodePointer@4
__imp__DecodePointer@4          dd _xyzzyDecodePointer@4

endif ; VCVER ge 10
endif ; VCVER

end

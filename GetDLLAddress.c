#include <windows.h>
#include <winternl.h>
#include <stdio.h>

void * DLLViaPEB(wchar_t * DllNameToSearch){

    PPEB pPeb = 0;
	PLDR_DATA_TABLE_ENTRY pDataTableEntry = 0;
    PVOID ntdllAddress = 0;

	//Retrieve from the TEB (Thread Environment Block) the PEB (Process Environment Block) address
    #ifdef _M_X64
        //If 64 bits architecture
        PPEB pPEB = (PPEB) __readgsqword(0x60);
    #else
        //If 32 bits architecture
        PPEB pPEB = (PPEB) __readfsdword(0x30);
    #endif

	//Retrieve the PEB_LDR_DATA address
	PPEB_LDR_DATA pLdr = pPEB->Ldr;

	//Address of the First PLIST_ENTRY Structure
    PLIST_ENTRY AddressFirstPLIST = &pLdr->InMemoryOrderModuleList;

	//Address of the First Module which is always (I think) the current program;
	PLIST_ENTRY AddressFirstNode = AddressFirstPLIST->Flink;

    //Searching through all module the DLL we want
	for (PLIST_ENTRY Node = AddressFirstNode; Node != AddressFirstPLIST ;Node = Node->Flink) // Node = Node->Flink means we go the next Node !
	{
		// Node is pointing to InMemoryOrderModuleList in the LDR_DATA_TABLE_ENTRY structure.
        // InMemoryOrderModuleList is at the second position in this structure.
		// To cast in the proper type, we need to go at the start of the structure.
        // To do so, we need to subtract 1 byte. Indeed, InMemoryOrderModuleList is at 0x008 from the start of the structure) 
		Node = Node - 1;

        // DataTableEntry structure
		pDataTableEntry = (PLDR_DATA_TABLE_ENTRY)Node;

        // Retrieve de full DLL Name from the DataTableEntry
        wchar_t * FullDLLName = (wchar_t *)pDataTableEntry->FullDllName.Buffer;

        //We lower the full DLL name for comparaison purpose
        for(int size = wcslen(FullDLLName), cpt = 0; cpt < size ; cpt++){
            FullDLLName[cpt] = tolower(FullDLLName[cpt]);
        }

        // We check if the full DLL name is the one we are searching
        // If yes, return  the dll base address
        if(wcsstr(FullDLLName, DllNameToSearch) != NULL){
            ntdllAddress = (PVOID)pDataTableEntry->DllBase;
            return ntdllAddress;
        }

		// Now, We need to go at the original position (InMemoryOrderModuleList), to be able to retrieve the next Node with ->Flink
		Node = Node + 1;
	}

    return ntdllAddress;
}
void main()
{
    wchar_t * DllNameToSearch = L"ntdll";

    PVOID address = DLLViaPEB(DllNameToSearch);
    if(address){
        printf("\n Here we go the address of  %ls : %x", DllNameToSearch, address);
    }else{
        printf("\n Address not found :(");
    }
    
}

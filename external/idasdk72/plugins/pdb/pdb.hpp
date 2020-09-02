
#ifndef PDB_HPP
#define PDB_HPP

#define PDB_NODE_NAME             "$ pdb"
#define PDB_DLLBASE_NODE_IDX       0
#define PDB_DLLNAME_NODE_IDX       0
#define PDB_LOADING_WIN32_DBG      1
#define PDB_TYPESONLY_NODE_IDX     2

enum pdb_callcode_t
{
  // user invoked 'load pdb' command, load pdb for the input file.
  // after invocation, result (boolean) is stored in: netnode(PDB_NODE_NAME).altval(PDB_DLLBASE_NODE_IDX)
  PDB_CC_USER = 0,
  // ida decided to call the plugin itself
  PDB_CC_IDA  = 1,
  // load additional pdb. This is semantically the same as
  // PDB_CC_USER (i.e., "File > Load file > PDB file..."), except
  // it won't ask the user for the data; rather it expects it in
  // netnode(PDB_NODE_NAME):
  //   load_addr: netnode(PDB_NODE_NAME).altval(PDB_DLLBASE_NODE_IDX)
  //   dll_name:  netnode(PDB_NODE_NAME).supstr(PDB_DLLNAME_NODE_IDX)
  PDB_CC_USER_WITH_DATA = 3,
};

#endif

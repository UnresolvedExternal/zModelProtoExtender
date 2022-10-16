// Supported with union (c) 2020 Union team

// User API for zCModel
// Add your methods here

int GetAniIDFromAniName_Union( zSTRING const& ) const;
int ApplyModelProtoOverlay_Union( const zSTRING& mdsFileName );
int ApplyModelProtoOverlay_Union2( zCModelPrototype* );
void RemoveModelProtoOverlay_Union( zCModelPrototype* );
void RemoveModelProtoOverlayByName_Union( zSTRING const& );
zCModelAniActive* GetActiveAni( const zSTRING& aniName );
void ActivateAdditionalAnis( zCModelPrototype* modelProto );
bool DeactivateAdditionalAnis( zCModelPrototype* modelProto );
void CheckAndApplyModelContext( zCModelPrototype* modelProto );
void RemoveFromAniHistory( zCModelPrototype* modelProto );
void RenameModelAni( zCModelPrototype* modelProto, zSTRING oldName, zSTRING newName );
void CopyProtoNodeList_Union();
void Destructor();
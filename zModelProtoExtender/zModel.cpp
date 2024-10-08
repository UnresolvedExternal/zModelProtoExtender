// Supported with union (c) 2020 Union team
// Union SOURCE file

#include <unordered_map>

namespace GOTHIC_ENGINE {
  HOOK Hook_zCModel_GetAniIDFromAniName_Union PATCH( &zCModel::GetAniIDFromAniName, &zCModel::GetAniIDFromAniName_Union );

  int zCModel::GetAniIDFromAniName_Union( const zSTRING& aniName ) const {
    for( int i = 0; i < modelProtoList.GetNum(); i++ ) {
      zCModelAni* ani = modelProtoList[0]->SearchAni( aniName );
      if( ani )
        return ani->aniID;
    }

    return Invalid;
  }


  void zCModel::CorrectAnisOrder() {
    for( int i = 0; i < numActiveAnis - 1; i++ )
      if( zCModelAniActive*& leftAni = aniChannels[i] )
        for( int k = i + 1; k < numActiveAnis; k++ )
          if( zCModelAniActive*& rightAni = aniChannels[k] )
            if( leftAni->protoAni->aniID == rightAni->protoAni->aniID )
              if( leftAni->protoAni == rightAni->protoAni ) {
                if( leftAni->isFadingOut && !rightAni->isFadingOut )
                  std::swap( leftAni, rightAni );
              } else {
                bool swapped = false;

                for( int m = modelProtoList.GetNum() - 1; m >= 0; m-- )
                  if( modelProtoList[m]->protoAnis.GetNum() > leftAni->protoAni->aniID )
                    if( zCModelAni* protoAni = modelProtoList[m]->protoAnis[leftAni->protoAni->aniID] )
                      if( rightAni->protoAni == protoAni ) {
                        std::swap( leftAni, rightAni );
                        swapped = true;
                        break;
                      }
                 
                if( !swapped )
                  if( leftAni->isFadingOut && !rightAni->isFadingOut )
                    std::swap( leftAni, rightAni );
              }
  }


  HOOK Hook_zCModel_ApplyModelProtoOverlay PATCH( &zCModel::ApplyModelProtoOverlay, &zCModel::ApplyModelProtoOverlay_Union );

  int zCModel::ApplyModelProtoOverlay_Union( const zSTRING& mdsFileName ) {
    if( modelProtoList.GetNum() == 0 )
      return False;

    CurrentModel = this;

    zCModelPrototype* modelProto = zCModelPrototype::Load( mdsFileName, modelProtoList[0] );

    bool_t Ok = False;
    if( modelProto ) {
      Ok = ApplyModelProtoOverlay_Union2( modelProto );
      modelProto->Release();
    }

    CurrentModel = Null;
    return Ok;
  };

  HOOK Hook_zCModel_ApplyModelProtoOverlay_2 PATCH( &zCModel::ApplyModelProtoOverlay, &zCModel::ApplyModelProtoOverlay_Union2 );

  int zCModel::ApplyModelProtoOverlay_Union2( zCModelPrototype* modelProto ) {
    bool ok = THISCALL( Hook_zCModel_ApplyModelProtoOverlay_2 )( modelProto );

    if( ok ) {
      ActivateAdditionalAnis( modelProto );
      CorrectAnisOrder();
    }

    return ok;
  }

  HOOK Hook_oCNpc_ApplyOverlay PATCH( &oCNpc::ApplyOverlay, &oCNpc::ApplyOverlay_Union );

  int oCNpc::ApplyOverlay_Union( const zSTRING& mdsFileName__ ) {

    string mdsFileName = mdsFileName__;
    mdsFileName.Upper();
    if( !mdsFileName.EndWith( ".MDS" ) )
      mdsFileName += ".MDS";

    if( visual && !GetModel()->ApplyModelProtoOverlay_Union( mdsFileName ) )
        return False;

    activeOverlays.Insert( mdsFileName );
    return True;
  }




  HOOK Hook_zCModel_RemoveModelProtoOverlay PATCH( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlay_Union );

  void zCModel::RemoveModelProtoOverlay_Union( zCModelPrototype* modelProto ) {
    CurrentModel = this; 

    if( modelProto )
      modelProto->AddRef();

    const bool isIn = modelProto && modelProtoList.IsInList( modelProto );
    THISCALL( Hook_zCModel_RemoveModelProtoOverlay )( modelProto );

    if (isIn && !modelProtoList.IsInList( modelProto )) {
      RemoveFromAniHistory( modelProto );

      // Hmmm, this overlay will works a some
      // seconds for a 'soft' anis replacing.
      if( !DeactivateAdditionalAnis( modelProto ) )
        modelProto->DelayedRelease( this );
      else
        modelProto->Release();

      CorrectAnisOrder();
    }

    CurrentModel = Null;
  }




  HOOK Hook_zCModel_RemoveModelProtoOverlayByName PATCH( &zCModel::RemoveModelProtoOverlay, &zCModel::RemoveModelProtoOverlayByName_Union );

  // So... This function has inline of
  // overload with 'zCModelPrototype*', fix it...
  void zCModel::RemoveModelProtoOverlayByName_Union( const zSTRING& fileName ) {
    zPATH filePath       = fileName;
    zSTRING realFileName = filePath.GetFilename();

    for( int i = 0; i < modelProtoList.GetNumInList(); i++ )
      if( modelProtoList[i]->modelProtoName == realFileName )
        return RemoveModelProtoOverlay_Union( modelProtoList[i] );
  }

  class AniStarter
  {
  private:
    struct AniStartInfo
    {
      zCModelAni* protoAni;
      float progress;

      void ModifyAni( zCModelAniActive* activeAni ) const {
        activeAni->SetProgressPercent_Union( progress );
      }

      void Init( zCModelAniActive* activeAni ) {
        progress = activeAni->GetProgressPercent();
      }
    };

    zCModel& model;
    std::vector<AniStartInfo> anis;

  public:
    AniStarter(zCModel* model) : model(*model) {
    }

    void Enqueue( zCModelAniActive* activeAni, zCModelAni* newAni = Null ) {
      if( zCModelAni* protoAni = newAni ? newAni : model.GetAniFromAniID( model.GetAniIDFromAniName( activeAni->protoAni->aniName ) ) ) {
        anis.emplace_back();
        AniStartInfo& info = anis.back();
        info.protoAni = protoAni;
        info.Init( activeAni );
      }
    }

    ~AniStarter() {
      for each( const AniStartInfo& info in anis ) {
        model.StartAni( info.protoAni, zCModel::zMDL_STARTANI_FORCE );

        if( zCModelAniActive* activeAni = model.GetActiveAni( info.protoAni ) )
          info.ModifyAni( activeAni );
      }
    }
  };

  // 'Soft' overlay adding with
  // replacing all active animations
  // to new overlay anis
  void zCModel::ActivateAdditionalAnis( zCModelPrototype* modelProto ) {
    Array<zCModelAni**> anis;

    for( int i = 0; i < numActiveAnis; i++ ) {
      anis.InsertEnd( &aniChannels[i]->nextAni );
      anis.InsertEnd( &aniChannels[i]->nextAniOverride );
    }

    for each( zCModelAni** nextAni in anis )
      if( zCModelAni*& ani = *nextAni )
        if( zCModelAni* newAni = modelProto->SearchAni( ani->aniName ) )
          ani = newAni;

    AniStarter aniStarter(this);

    for( int i = 0; i < numActiveAnis; i++ )
      if( zCModelAniActive* activeAni = aniChannels[i] )
        if( !activeAni->isFadingOut ) {
          zCModelAni* newAni = modelProto->SearchAni( activeAni->protoAni->aniName );

          if( activeAni->protoAni == newAni || !newAni )
            continue;

          aniStarter.Enqueue( activeAni, newAni );
          FadeOutAni( activeAni );
        }
  }



  // 'Soft' overlay removing with
  // replacing all active ovrerlay
  // animations to bottom-level anis
  bool zCModel::DeactivateAdditionalAnis( zCModelPrototype* modelProto ) {
    bool instant = true;
    AniStarter aniStarter(this);

    for( int i = 0; i < numActiveAnis; i++ )
      if( zCModelAniActive* activeAni = aniChannels[i] )
        if( activeAni->protoAni == modelProto->SearchAni( activeAni->protoAni->aniName ) ) {
          if( !activeAni->isFadingOut ) {
            FadeOutAni( activeAni );
            aniStarter.Enqueue( activeAni );
          }

          instant = false;
        }

    Array<zCModelAni**> anis;
    anis.Clear();

    for( int i = 0; i < numActiveAnis; i++ ) {
      anis.InsertEnd( &aniChannels[i]->nextAni );
      anis.InsertEnd( &aniChannels[i]->nextAniOverride );
    }

    for each( zCModelAni** nextAni in anis )
      if( zCModelAni*& ani = *nextAni )
        if( zCModelAni* newAni = modelProto->SearchAni( ani->aniName) )
          if( ani == newAni ) {
            const zSTRING aniName = ani->aniName;
            ani = Null;

            for( int i = modelProtoList.GetNum() - 1; i >= 0; i-- )
              if ( zCModelAni* protoAni = modelProtoList[i]->SearchAni( aniName ) ) {
                ani = protoAni;
                break;
              }
          }

    return instant;
  }

  class FakeAniHistoryPool
  {
  private:
    int nextIndex;
    zSTRING aniNames[MAX_ANIHISTORY];

    zCModelAni* GetFakeAni( int index ) {
      return reinterpret_cast<zCModelAni*>( reinterpret_cast<char*>( &aniNames[index] ) - 0x24 );
    }

  public:
    zCModelAni* CreateFakeAni( zCModelAni* protoAni ) {
      if( nextIndex < 0 || nextIndex >= MAX_ANIHISTORY )
        nextIndex = 0;

      aniNames[nextIndex] = protoAni->aniName;
      return GetFakeAni( nextIndex++ );
    }
  };

  std::unordered_map<zCModel*, FakeAniHistoryPool> fakeAniPools;

  void zCModel::RemoveFromAniHistory( zCModelPrototype* modelProto ) {
    if( aniHistoryList )
      for( int i = 0; i < MAX_ANIHISTORY; i++ )
        if( zCModelAni*& ani = aniHistoryList[i] )
          if( modelProto->SearchAni( ani->aniName ) == ani )
            ani = fakeAniPools[this].CreateFakeAni( ani );
  }

  // Find active animation by name
  zCModelAniActive* zCModel::GetActiveAni( const zSTRING& aniName ) {
    for( int i = 0; i < numActiveAnis; i++ )
      if( aniChannels[i]->protoAni->aniName == aniName )
        return aniChannels[i];
    
    return Null;
  }

  HOOK Hook_zCModel_CopyProtoNodeList PATCH( &zCModel::CopyProtoNodeList, &zCModel::CopyProtoNodeList_Union );

  void zCModel::CopyProtoNodeList_Union() {
    if( modelProtoList.GetNum() <= 0 || !modelProtoList[0] )
      return;

    int num = modelProtoList[0]->nodeList.GetNum();

    for( int i = 0; i < num; i++ ) {
      zCModelNode* node = modelProtoList[0]->nodeList[i];
      zCModelNode* parent = node->parentNode;
      zCModelNodeInst* instNode = new zCModelNodeInst( node );
      if( parent )
        instNode->parentNode = SearchNode( parent->nodeName );

      if( instNode->protoNode->nodeName == "" )
        cmd << "EMPTY NODE NAME!!!" << endl;

      node->lastInstNode = instNode;
      nodeList.Insert( instNode );
    }
  }


  // FIXME!!! Activate the first block after release of a 1.0k!!!
#if 0
  HOOK Hook_zCModel_Destructor PATCH( &zCModel::~zCModel, &zCModel::Destructor );
#else
#if ENGINE == Engine_G1
  HOOK Hook_zCModel_Destructor PATCH( 0x0055E4F0, &zCModel::Destructor );
#endif
#if ENGINE == Engine_G1A
  HOOK Hook_zCModel_Destructor PATCH( 0x00576880, &zCModel::Destructor );
#endif
#if ENGINE == Engine_G2
  HOOK Hook_zCModel_Destructor PATCH( 0x00572A10, &zCModel::Destructor );
#endif
#if ENGINE == Engine_G2A
  HOOK Hook_zCModel_Destructor PATCH( 0x00577CC0, &zCModel::Destructor );
#endif
#endif

  void zCModel::Destructor() {
    auto nodes = nodeList;
    THISCALL( Hook_zCModel_Destructor )();
    for( int i = 0; i < nodes.GetNum(); i++ )
      if( nodes[i] )
        delete nodes[i];

    fakeAniPools.erase( this );
  };
}
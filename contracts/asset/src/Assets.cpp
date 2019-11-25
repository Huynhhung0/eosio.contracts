#include <Assets.hpp>

ACTION Assets::cleartables1() {
  require_auth(_self);
  simagedigests st(_self, _self.value);
  auto itr = st.begin();
  while(itr != st.end()) {
    itr = st.erase(itr);
  }
}

/*
ACTION Assets::genid(name submitted_by, string r) {
	require_auth(submitted_by);
	check( is_account( submitted_by ), "submitted_by account does not exist." );
	require_recipient( submitted_by );
	checksum256 r256 = sha256(r.c_str(), r.size() * sizeof(char));
	auto checksumBytes = r256.extract_as_byte_array().data();
	uint64_t newID;
	memcpy(&newID, &checksumBytes[7], 8);

	//Events
	sendEvent( submitted_by, submitted_by, "saenewasset"_n, std::make_tuple( submitted_by, checksumBytes ) );
	SEND_INLINE_ACTION( *this, newassetlog, { {_self, "active"_n} }, { submitted_by, &checksumBytes[7]} );

}
*/

ACTION Assets::cleartables2() {
  require_auth(_self);
  stextdigests st1(_self, _self.value);
  auto itr1 = st1.begin();
  while(itr1 != st1.end()) {
    itr1 = st1.erase(itr1);
  }
}

ACTION Assets::updatever( string version ) {

	require_auth( get_self() );
	Configs configs( _self, _self.value );
	configs.set( tokenconfigs{ "simpleassets"_n, version }, _self );
}

ACTION Assets::regsubmitted( name submitted_by, string data, string stemplate, string imgpriority ) {

	require_auth( submitted_by );
	require_recipient( submitted_by );
	check( data.size() > 3, "Data field is too short. Please tell us about yourselves." );
	submitted_bys submitted_by_( _self, _self.value );

	if ( submitted_by_.find( submitted_by.value ) == submitted_by_.end() ) {
		submitted_by_.emplace( submitted_by, [&]( auto& s ) {
			s.submitted_by = submitted_by;
			s.data = data;
			s.stemplate = stemplate;
			s.imgpriority = imgpriority;
		});
	}
	else {
		check( false, "Registration Error. You're probably already registered. Try the authupdate action." );
	}
}

ACTION Assets::submittedud( name submitted_by, string data, string stemplate, string imgpriority ) {

	require_auth( submitted_by );
	require_recipient( submitted_by );
	submitted_bys submitted_by_( _self, _self.value );
	auto itr = submitted_by_.find( submitted_by.value );
	check( itr != submitted_by_.end(), "submitted_by not registered" );

	if ( data.empty() && stemplate.empty() ) {
		itr = submitted_by_.erase( itr );
	}
	else {
		submitted_by_.modify( itr, submitted_by, [&]( auto& s ) {
			s.data = data;
			s.stemplate = stemplate;
			s.imgpriority = imgpriority;
		});
	}
}

//@TODO dont allow an submitted_by run brute force new asset_id.
ACTION Assets::newasset(name submitted_by) {
	require_auth(submitted_by);
	check( is_account( submitted_by ), "submitted_by account does not exist." );
	require_recipient( submitted_by );
	const auto newID = getid();
	sassets assets( _self, submitted_by.value );
	string empty = "";
	bool no = false;

	assets.emplace( _self, [&]( auto& s ) {
		s.id = newID;
		s.platform = submitted_by;
		s.submitted_by = submitted_by;
	});

	//Events
	sendEvent( submitted_by, submitted_by, "saenewasset"_n, std::make_tuple( submitted_by, newID ) );
	SEND_INLINE_ACTION( *this, newassetlog, { {_self, "active"_n} }, { submitted_by, newID} );

}

ACTION Assets::create( name submitted_by, uint64_t asset_id, string idata, string mdata, string common_info, string detail_info, string ref_info) {
	require_auth( submitted_by );
	sassets assets_f( _self, submitted_by.value );
	const auto itrAsset = assets_f.find( asset_id );
	check( itrAsset != assets_f.end(), "asset not found" );
	check( itrAsset->submitted_by == submitted_by, "Only submitted_by can update asset." );
	check( itrAsset->idata.compare("") == 0, "Can not update asset data." );
	check(json::accept(idata), "invalid idata json.");
	check(json::accept(mdata), "invalid mdata json.");
	check(json::accept(common_info), "invalid common_info json.");
	check(json::accept(detail_info), "invalid detail_info json.");
	check(json::accept(ref_info), "invalid ref_info json.");
	name platform = submitted_by;
	json js = json::parse(idata);	
	string digestString = js["digest"].get<string>();
	string type = js["type"].get<string>();
	bool isDuplicate;
	std::vector<uint64_t> duplicateAssetIDs;
	std::vector<checksum256> digestsForInsert;
	std::vector<std::vector<checksum256>> buckets = getBucket(digestString, type);
	std::tie(isDuplicate, duplicateAssetIDs, digestsForInsert ) = checkDuplicate(buckets, type);
	if(isDuplicate) {
		string msg = "found duplicate digest with Asset IDs: ";
	    for (int i = 0; i < duplicateAssetIDs.size(); i++) {
			msg = msg + std::to_string(duplicateAssetIDs[i]);
			if (i != (duplicateAssetIDs.size() -1) ) msg = msg + ", ";
		}
		check(false, msg);
	} else {
	  for (int i =0; i < digestsForInsert.size(); i++) {
        if (type.compare("TEXT") == 0) {
		  stextdigests digests_f(_self, _self.value);
	      auto digest_index = digests_f.get_index<name("digest")>();
		  digests_f.emplace( _self, [&]( auto& d ) { d.id = getid("TEXT"); d.digest= digestsForInsert[i]; d.asset_id = asset_id;});
		} else if (type.compare("IMAGE") == 0) {
		  simagedigests digests_f(_self, _self.value);
	      auto digest_index = digests_f.get_index<name("digest")>();
		  digests_f.emplace( _self, [&]( auto& d ) { d.id = getid("IMAGE"); d.digest= digestsForInsert[i]; d.asset_id = asset_id;});
		}
	  }
	}
	assets_f.modify( itrAsset, submitted_by, [&]( auto& a ) {
		a.platform = platform;
		a.mdata = mdata; // mutable data
		a.idata = idata; // immutable data
		a.common_info = common_info;
		a.detail_info = detail_info;
		a.ref_info = ref_info;
	});

	//Events
	sendEvent( submitted_by, submitted_by, "saecreate"_n, std::make_tuple( platform, asset_id) );
	SEND_INLINE_ACTION( *this, createlog, { {_self, "active"_n} }, { submitted_by, platform, idata, mdata, common_info, detail_info, ref_info, asset_id } );
}

ACTION Assets::newassetlog( name submitted_by, uint64_t asset_id) {

	require_auth(get_self());
}

ACTION Assets::createlog( name submitted_by, name platform, string idata, string mdata, string common_info, string detail_info, string ref_info, uint64_t asset_id ) {

	require_auth(get_self());
}

ACTION Assets::claim( name claimer, std::vector<uint64_t>& asset_ids ) {

	require_auth( claimer );
	require_recipient( claimer );
	offers offert( _self, _self.value );
	sassets assets_t( _self, claimer.value );

	std::map< name, std::map< uint64_t, name > > uniqsubmitted_by;
	for ( auto i = 0; i < asset_ids.size(); ++i ) {
		auto itrc = offert.find( asset_ids[i] );
		check( itrc != offert.end(), "Cannot find at least one of the assets you're attempting to claim." );
		check( claimer == itrc->offeredto, "At least one of the assets has not been offerred to you." );

		sassets assets_f( _self, itrc->platform.value );
		auto itr = assets_f.find( asset_ids[i] );
		check( itr != assets_f.end(), "Cannot find at least one of the assets you're attempting to claim." );
		check( itrc->platform.value == itr->platform.value, "Owner was changed for at least one of the items!?" );

		assets_t.emplace( claimer, [&]( auto& s ) {
			s.id = itr->id;
			s.platform = claimer;
			s.submitted_by = itr->submitted_by;
			s.mdata = itr->mdata; 		// mutable data
			s.idata = itr->idata; 		// immutable data
			s.container = itr->container;
			s.containerf = itr->containerf;
		});

		//Events
		uniqsubmitted_by[itr->submitted_by][asset_ids[i]] = itrc->platform;

		assets_f.erase(itr);
		offert.erase(itrc);
	}

	for ( auto uniqsubmitted_byIt = uniqsubmitted_by.begin(); uniqsubmitted_byIt != uniqsubmitted_by.end(); ++uniqsubmitted_byIt ) {
		name keysubmitted_by = std::move( uniqsubmitted_byIt->first );
		sendEvent( keysubmitted_by, claimer, "saeclaim"_n, std::make_tuple( claimer, uniqsubmitted_by[keysubmitted_by] ) );
	}
}

ACTION Assets::transfer( name from, name to, string fromjsonstr, string tojsonstr, uint64_t asset_id, string memo ) {

	check( is_account( to ), "TO account does not exist" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	require_recipient( from );
	require_recipient( to );

	sassets assets_f( _self, from.value );
	sassets assets_t( _self, to.value );

	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );

	const auto rampayer = has_auth( to ) ? to : from;

	bool isDelegeting = false;

	check(json::accept(fromjsonstr), "from is invalid json.");
	check(json::accept(tojsonstr), "to is invalid json.");

	json fromjson = json::parse(fromjsonstr);
	json tojson = json::parse(tojsonstr);

	auto itrd = delegatet.find( asset_id );
	isDelegeting = false;
	if ( itrd != delegatet.end() ) {
		if ( itrd->platform == to || itrd->delegatedto == to ) {
			isDelegeting = true;
			if ( itrd->platform == to ) {
				delegatet.erase( itrd );
			}
		}
		else {
			check( false, "At least one of the assets cannot be transferred because it is delegated" );
		}
	}

	if ( isDelegeting ) {
		require_auth( has_auth( itrd->platform ) ? itrd->platform : from );
	}
	else {
		require_auth( from );
	}

	auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "At least one of the assets cannot be found (check ids?)" );
	check(!itr->revoke, "asset is revoked.");
	check( from.value == itr->platform.value, "At least one of the assets is not yours to transfer." );
	check( offert.find( asset_id ) == offert.end(), "At least one of the assets has been offered for a claim and cannot be transferred. Cancel offer?" );
	json validJSON = json::accept(itr->ref_info);
	check(validJSON, "ref_info is invalid json.");
	json refInfo = json::parse(itr->ref_info);
	string platformState = refInfo["owner"].get<string>();
	string refOwnerState = refInfo["ref_owner"].get<string>();
	if(!fromjson.empty()){
	  string fromOwner = fromjson["owner"].get<string>();
	  string fromRefOwner = fromjson["ref_owner"].get<string>();
	  check(platformState.compare(fromOwner) == 0, "cannot transfer from other owner.");
	  check(refOwnerState.compare(fromRefOwner) == 0, "cannot transfer from other ref_owner.");
	  if(!tojson.empty()) {
	    string toOwner = tojson["owner"].get<string>();
	    string toRefOwner = tojson["ref_owner"].get<string>();
	    check(platformState.compare(toOwner) != 0, "cannot transfer to yourself.");
	    check(refOwnerState.compare(toRefOwner) != 0, "cannot transfer to yourself.");
	    refInfo["owner"] = toOwner;
	    refInfo["ref_owner"] = toRefOwner;
	  }
	} 
	assets_f.erase(itr);
	assets_t.emplace( rampayer, [&]( auto& s ) {
		s.id = itr->id;
		s.platform = to;
		s.submitted_by = itr->submitted_by;
		s.idata = itr->idata; 		// immutable data
		s.ref_info= refInfo.dump(); 		   // mutable data
		s.container = itr->container;
		s.containerf = itr->containerf;

	});

	//Send Event as deferred
	sendEvent( itr->submitted_by, rampayer, "saetransfer"_n, std::make_tuple( from, to, asset_id, memo ) );
}

ACTION Assets::setmdata( name platform, uint64_t asset_id, string mdata ) {
	require_auth( platform );
	sassets assets_f( _self, platform.value );
	const auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "asset not found" );
	check(!itr->revoke, "asset is revoked.");
	check( itr->platform == platform, "Only platform can update asset." );

	json validJSON = json::accept(mdata);
	check(validJSON, "mdata is invalid json.");

	assets_f.modify( itr, platform, [&]( auto& a ) {
		a.mdata = mdata; 
	});

}

ACTION Assets::setdinfo( name platform, uint64_t asset_id, string detail_info) {
	require_auth( platform );
	sassets assets_f( _self, platform.value );
	const auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "asset not found" );
	check(!itr->revoke, "asset is revoked.");
	check( itr->platform == platform, "Only platform can update asset." );

	json validJSON = json::accept(detail_info);
	check(validJSON, "mdata is invalid json.");

	assets_f.modify( itr, platform, [&]( auto& a ) {
		a.detail_info = detail_info; 
	});

}


ACTION Assets::updatecinfo( name platform, uint64_t asset_id, string common_info) {

	require_auth( platform );
	sassets assets_f( _self, platform.value );
	const auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "asset not found" );
	check(!itr->revoke, "asset is revoked.");
	check( itr->platform == platform, "Only platform can update asset." );

	json validJSON = json::accept(common_info);
	check(validJSON, "mdata is invalid json.");
	json jsUpdate = json::parse(common_info);
	json jsMdata = json::parse(itr->common_info);
	jsMdata.merge_patch(jsUpdate);

	assets_f.modify( itr, platform, [&]( auto& a ) {
		a.common_info = jsMdata.dump();
	});
}

ACTION Assets::offer( name platform, name newplatform, std::vector<uint64_t>& asset_ids, string memo ) {

	check( platform != newplatform, "cannot offer to yourself" );
	require_auth( platform );
	require_recipient( platform );
	require_recipient( newplatform );
	check( is_account( newplatform ), "newplatform account does not exist" );

	sassets assets_f( _self, platform.value );
	offers offert( _self, _self.value );
	delegates delegatet( _self, _self.value );

	for ( auto i = 0; i < asset_ids.size(); ++i ) {
		check( assets_f.find( asset_ids[i] ) != assets_f.end(), "At least one of the assets was not found." );
		check( offert.find( asset_ids[i] ) == offert.end(), "At least one of the assets is already offered for claim." );
		check( delegatet.find( asset_ids[i] ) == delegatet.end(), "At least one of the assets is delegated and cannot be offered." );

		offert.emplace( platform, [&]( auto& s ) {
			s.asset_id = asset_ids[i];
			s.offeredto = newplatform;
			s.platform = platform;
			s.cdate = now();
		});
	}
}

ACTION Assets::canceloffer( name platform, std::vector<uint64_t>& asset_ids ) {

	require_auth( platform );
	require_recipient( platform );
	offers offert( _self, _self.value );

	for ( auto i = 0; i < asset_ids.size(); ++i ) {
		auto itr = offert.find( asset_ids[i] );
		check( itr != offert.end(), "The offer for at least one of the assets was not found." );
		check( platform.value == itr->platform.value, "You're not the platform of at least one of the assets whose offers you're attempting to cancel." );
		offert.erase( itr );
	}
}

ACTION Assets::revoke( name platform, uint64_t asset_id, string memo ) {

	require_auth( platform );
	sassets assets_f( _self, platform.value );
	stextdigests tdigests_f(_self, _self.value);
	simagedigests idigests_f(_self, _self.value);
	offers offert( _self, _self.value );
	delegates delegatet( _self, _self.value );


	auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "At least one of the assets was not found." );
	check( platform.value == itr->platform.value, "At least one of the assets you're attempting to revoke is not yours." );
	check( offert.find( asset_id ) == offert.end(), "At least one of the assets has an open offer and cannot be revokeed." );
	check( delegatet.find( asset_id ) == delegatet.end(), "At least one of assets is delegated and cannot be revokeed." );
	check( !itr->revoke , "Asset is already revoked." );

	json js = json::parse(itr->idata);	
	   string digestString;
	   string type;
	   for (auto& [key, value] : js.items()) {
	   	if (key.compare("type") == 0) { 
	   		type = value;
	   	}
	   }
	   if (type.compare("TEXT") == 0) {
	  while (true){ 
          auto idx = tdigests_f.get_index<name("asset")>();
	   auto itr = idx.find(asset_id);
	   if(itr == idx.end()) {
		   break;
	   }
	   idx.erase(itr);
	  }
	   } else if (type.compare("IMAGE") == 0){
	  while (true){ 
           auto idx = idigests_f.get_index<name("asset")>();
	    auto itr = idx.find(asset_id);
	    if(itr == idx.end()) {
	  	  break;
	    }
	    idx.erase(itr);
	  }
	   }

	assets_f.modify( itr, platform, [&]( auto& a ) {
		a.revoke = true;
	});

	//Send Event as deferred
	sendEvent( itr->submitted_by, platform, "saerevoke"_n, std::make_tuple( platform, asset_id, memo ) );
}

ACTION Assets::delegate( name platform, name to,string fromjson, string tojson, uint64_t asset_id, uint64_t period, string memo ) {

	check(memo.size() <= 64, "Error. Size of memo cannot be bigger 64");
	check( platform != to, "cannot delegate to yourself" );
	require_auth( platform );
	require_recipient( platform );
	check( is_account( to ), "TO account does not exist" );

	sassets assets_f( _self, platform.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );

	check( assets_f.find( asset_id ) != assets_f.end(), "At least one of the assets cannot be found." );
	check( delegatet.find( asset_id ) == delegatet.end(), "At least one of the assets is already delegated." );
	check( offert.find( asset_id ) == offert.end(), "At least one of the assets has an open offer and cannot be delegated." );

	delegatet.emplace( platform, [&]( auto& s ) {
		s.asset_id = asset_id;
		s.platform = platform;
		s.delegatedto = to;
		s.cdate = now();
		s.period = period;
		s.memo = memo;
	});

	transfer( platform, to, fromjson, tojson, asset_id, "Delegate memo: " + memo );
}

ACTION Assets::delegatemore( name platform, uint64_t asset_idc, uint64_t period ) {

	require_auth( platform );
	require_recipient( platform );

	delegates delegatet( _self, _self.value );
	const auto itrc = delegatet.find( asset_idc );
	check( itrc != delegatet.end(), "Assets asset_idc is not delegated." );
	
	delegatet.modify( itrc, platform, [&]( auto& s ) {
		s.period = itrc->period + period;
	});
}

ACTION Assets::undelegate( name platform, name from,string fromjson, string tojson, uint64_t asset_id ) {

	require_auth( platform );
	require_recipient( platform );
	check( is_account( from ), "to account does not exist" );

	sassets assets_f( _self, from.value );
	delegates delegatet( _self, _self.value );

	auto itr = assets_f.find( asset_id );
	check( itr != assets_f.end(), "At least one of the assets cannot be found." );
	auto itrc = delegatet.find( asset_id );
	check( itrc != delegatet.end(), "At least one of the assets is not delegated." );
	check( platform == itrc->platform, "You are not the platform of at least one of these assets." );
	check( from == itrc->delegatedto, "FROM does not match DELEGATEDTO for at least one of the assets." );
	check( itr->platform == itrc->delegatedto, "FROM does not match DELEGATEDTO for at least one of the assets." );
	check( ( itrc->cdate + itrc->period ) < now(), "Cannot undelegate until the PERIOD expires." );

	string asset_idsmemo = std::to_string( asset_id );

	transfer( from, platform, fromjson, tojson, asset_id, "undelegate asset_id: " + asset_idsmemo );
}


ACTION Assets::attach( name platform, uint64_t asset_idc, std::vector<uint64_t>& asset_ids ) {

	sassets assets_f( _self, platform.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );
	require_recipient( platform );
	const auto ac_ = assets_f.find( asset_idc );
	check( ac_ != assets_f.end(), "Asset cannot be found." );
	require_auth( ac_->submitted_by );

	for ( auto i = 0; i < asset_ids.size(); ++i ) {
		auto itr = assets_f.find( asset_ids[i] );
		check( itr != assets_f.end(), "At least one of the assets cannot be found." );
		check( asset_idc != asset_ids[i], "Cannot attcach to self." );
		check( itr->submitted_by == ac_->submitted_by, "Different submitted_bys." );
		check( delegatet.find( asset_ids[i] ) == delegatet.end(), "At least one of the assets is delegated." );
		check( offert.find( asset_ids[i] ) == offert.end(), "At least one of the assets has an open offer and cannot be delegated." );

		assets_f.modify( ac_, ac_->submitted_by, [&]( auto& a ) {
			a.container.push_back( *itr );
		});
		assets_f.erase( itr );
	}
}

ACTION Assets::detach( name platform, uint64_t asset_idc, std::vector<uint64_t>& asset_ids ) {

	require_auth( platform );
	require_recipient( platform );
	sassets assets_f( _self, platform.value );

	const auto ac_ = assets_f.find( asset_idc );
	check( ac_ != assets_f.end(), "Asset cannot be found." );

	delegates delegatet( _self, _self.value );
	check( delegatet.find( asset_idc ) == delegatet.end(), "Cannot detach from delegated. asset_idc is delegated." );

	for ( auto i = 0; i < asset_ids.size(); ++i ) {
		std::vector<sasset> newcontainer;

		for ( auto j = 0; j < ac_->container.size(); ++j ) {
			auto acc = ac_->container[j];
			if ( asset_ids[i] == acc.id ) {
				assets_f.emplace( platform, [&]( auto& s ) {
					s.id = acc.id;
					s.platform = platform;
					s.submitted_by = acc.submitted_by;
					s.idata = acc.idata; 		// immutable data
					s.mdata = acc.mdata; 		// mutable data
					s.container = acc.container;
					s.containerf = acc.containerf;
				});
			}
			else {
				newcontainer.push_back( acc );
			}
		}

		assets_f.modify( ac_, platform, [&]( auto& a ) {
			a.container = newcontainer;
		});
	}
}

ACTION Assets::attachf( name platform, name submitted_by, asset quantity, uint64_t asset_idc ) {

	attachdeatch( platform, submitted_by, quantity, asset_idc, true );
}


ACTION Assets::detachf( name platform, name submitted_by, asset quantity, uint64_t asset_idc ) {

	attachdeatch( platform, submitted_by, quantity, asset_idc, false );
}

ACTION Assets::createf( name submitted_by, asset maximum_supply, bool submitted_byctrl, string data ) {

	require_auth( submitted_by );
	const auto sym = maximum_supply.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( maximum_supply.is_valid(), "invalid supply" );
	check( maximum_supply.amount > 0, "max-supply must be positive" );

	stats statstable( _self, submitted_by.value );
	check( statstable.find( sym.code().raw() ) == statstable.end(), "token with symbol already exists" );

	statstable.emplace( submitted_by, [&]( auto& s ) {
		s.supply.symbol = maximum_supply.symbol;
		s.max_supply = maximum_supply;
		s.issuer = submitted_by;
		s.id = getid();
		s.submitted_byctrl = submitted_byctrl;
		s.data = data;
	});
}

ACTION Assets::updatef( name submitted_by, symbol sym, string data ) {

	require_auth( submitted_by );
	check( sym.is_valid(), "invalid symbol name" );
	stats statstable( _self, submitted_by.value );
	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "Symbol not exists" );

	statstable.modify( existing, submitted_by, [&]( auto& a ) {
		a.data = data;
	});
}

ACTION Assets::issuef( name to, name submitted_by, asset quantity, string memo ) {

	const auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	stats statstable( _self, submitted_by.value );
	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );

	require_auth( existing->issuer );
	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must issue positive quantity" );

	check( quantity.symbol == existing->supply.symbol, "symbol precision mismatch" );
	check( quantity.amount <= existing->max_supply.amount - existing->supply.amount, "quantity exceeds available supply" );

	statstable.modify( *existing, same_payer, [&]( auto& s ) {
		s.supply += quantity;
	});

	add_balancef( existing->issuer, submitted_by, quantity, existing->issuer );

	if ( to != existing->issuer ) {
		transferf( existing->issuer, to, submitted_by, quantity, memo );
	}
}

ACTION Assets::transferf( name from, name to, name submitted_by, asset quantity, string memo ) {

	check( from != to, "cannot transfer to self" );
	check( is_account( to ), "to account does not exist" );
	const auto sym = quantity.symbol.code();
	stats statstable( _self, submitted_by.value );
	const auto& st = statstable.get( sym.raw() );

	require_recipient( from );
	require_recipient( to );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must transfer positive quantity" );
	check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	auto payer = has_auth( to ) ? to : from;
	auto checkAuth = from;

	if ( st.submitted_byctrl && has_auth( st.issuer ) ) {
		checkAuth = st.issuer;
		payer = st.issuer;
	}

	require_auth( checkAuth );
	sub_balancef( from, submitted_by, quantity );
	add_balancef( to, submitted_by, quantity, payer );
}

ACTION Assets::offerf( name platform, name newplatform, name submitted_by, asset quantity, string memo ) {

	require_auth( platform );
	require_recipient( platform );
	require_recipient( newplatform );
	check( is_account( newplatform ), "newplatform account does not exist" );
	check( platform != newplatform, "cannot offer to yourself" );
	const auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	stats statstable( _self, submitted_by.value );
	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist" );
	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must retire positive quantity" );
	check( quantity.symbol == existing->supply.symbol, "symbol precision mismatch" );

	offerfs offert( _self, _self.value );
	auto platform_index = offert.template get_index< "platform"_n >();

	for ( auto itro = platform_index.find( platform.value ); itro != platform_index.end(); itro++ ) {
		check( !( itro->submitted_by == submitted_by && itro->offeredto == newplatform && itro->quantity.symbol == quantity.symbol ), "Such an offer already exists" );
	}

	offert.emplace( platform, [&]( auto& s ) {
		s.id = getid("DEFER");
		s.submitted_by = submitted_by;
		s.quantity = quantity;
		s.offeredto = newplatform;
		s.platform = platform;
		s.cdate = now();
	});
	sub_balancef( platform, submitted_by, quantity );
}

ACTION Assets::cancelofferf( name platform, std::vector<uint64_t>& ftofferids ) {

	require_auth( platform );
	require_recipient( platform );
	offerfs offert( _self, _self.value );

	for ( auto i = 0; i < ftofferids.size(); ++i ) {
		auto itr = offert.find( ftofferids[i] );
		check( itr != offert.end(), "The offer for at least one of the FT was not found." );
		check( platform.value == itr->platform.value, "You're not the platform of at least one of those FTs." );
		add_balancef( platform, itr->submitted_by, itr->quantity, platform );
		offert.erase( itr );
	}
}

ACTION Assets::claimf( name claimer, std::vector<uint64_t>& ftofferids ) {

	require_auth( claimer );
	require_recipient( claimer );
	offerfs offert( _self, _self.value );
	std::map< name, std::vector< uint64_t > > uniqsubmitted_by;

	for ( auto i = 0; i < ftofferids.size(); ++i ) {
		auto itrc = offert.find( ftofferids[i] );
		check( itrc != offert.end(), "Cannot find at least one of the FT you're attempting to claim." );
		check( claimer == itrc->offeredto, "At least one of the FTs has not been offerred to you." );
		add_balancef( claimer, itrc->submitted_by, itrc->quantity, claimer );
		offert.erase( itrc );
	}
}

ACTION Assets::revokef( name from, name submitted_by, asset quantity, string memo ) {

	auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );
	stats statstable( _self, submitted_by.value );

	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist" );
	require_auth( existing->submitted_byctrl && has_auth( existing->issuer ) ? existing->issuer : from );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must retire positive quantity" );
	check( quantity.symbol == existing->supply.symbol, "symbol precision mismatch" );

	statstable.modify( *existing, same_payer, [&]( auto& s ) {
		s.supply -= quantity;
	});

	sub_balancef( from, submitted_by, quantity );
}

ACTION Assets::openf( name platform, name submitted_by, const symbol& symbol, name ram_payer ) {

	require_auth( ram_payer );
	stats statstable( _self, submitted_by.value );
	const auto& st = statstable.get( symbol.code().raw(), "symbol does not exist" );
	check( st.supply.symbol == symbol, "symbol precision mismatch" );
	accounts acnts( _self, platform.value );

	if ( acnts.find( st.id ) == acnts.end() ) {
		acnts.emplace( ram_payer, [&]( auto& a ) {
			a.id = st.id;
			a.submitted_by = submitted_by;
			a.balance = asset{ 0, symbol };
		});
	}
}

ACTION Assets::closef( name platform, name submitted_by, const symbol& symbol ) {

	require_auth( platform );
	accounts acnts( _self, platform.value );
	auto it = acnts.find( getFTIndex( submitted_by, symbol ) );
	check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
	check( it->balance.amount == 0, "Cannot close because the balance is not zero." );

	offerfs offert( _self, _self.value );
	const auto platform_index = offert.template get_index< "platform"_n >();
	for ( auto itro = platform_index.find( platform.value ); itro != platform_index.end(); itro++ ) {
		check( !( itro->submitted_by == submitted_by && itro->quantity.symbol == symbol ), "You have open offers for this FT.." );
	}

	acnts.erase( it );
}

uint64_t Assets::getid( string type ) {

	// getid private action Increment, save and return id for a new asset or new fungible token.
	conf config( _self, _self.value );
	_cstate = config.exists() ? config.get() : global{};

	uint64_t resid;
	if ( type.compare("DEFER") == 0 ) {
		_cstate.defid++;
		resid = _cstate.defid;
	} else if (type.compare("TEXT") == 0) { 
		_cstate.textid++;
		resid = _cstate.textid;

	} else if ( type.compare("IMAGE") == 0) {
		_cstate.imageid++;
		resid = _cstate.imageid;
	}
	else { // asset
		_cstate.lnftid++;
		resid = _cstate.lnftid;
	}

	config.set( _cstate, _self );
	return resid;
}

uint64_t Assets::getFTIndex( name submitted_by, symbol symbol ) {

	stats statstable( _self, submitted_by.value );
	const auto existing = statstable.find( symbol.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist." );
	return existing->id;
}

void Assets::attachdeatch( name platform, name submitted_by, asset quantity, uint64_t asset_idc, bool attach ) {

	sassets assets_f( _self, platform.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );
	stats statstable( _self, submitted_by.value );
	const auto& st = statstable.get( quantity.symbol.code().raw() );

	require_recipient( platform );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must transfer positive quantity" );
	check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
	check( st.issuer == submitted_by, "Different submitted_bys" );

	if ( attach ) {
		require_auth( submitted_by );	 //attach
	}
	else {
		require_auth( platform );  //deatach
	}

	const auto itr = assets_f.find( asset_idc );
	check( itr != assets_f.end(), "asset_id cannot be found." );
	check( itr->submitted_by == submitted_by, "Different submitted_bys." );
	check( delegatet.find(asset_idc) == delegatet.end(), "Asset is delegated." );
	check( offert.find(asset_idc) == offert.end(), "Assets has an open offer and cannot be delegated." );

	std::vector<account> newcontainerf;
	bool found = false;

	for ( auto j = 0; j < itr->containerf.size(); j++ ) {
		auto accf = itr->containerf[j];
		if ( st.id == accf.id ) {
			if ( attach ) {
				accf.balance.amount += quantity.amount;
			}
			else {
				check( accf.balance.amount >= quantity.amount, "overdrawn balance" );
				accf.balance.amount -= quantity.amount;
			}
			found = true;
		}
		if ( accf.balance.amount > 0 )
			newcontainerf.push_back( accf );
	}

	if ( !found && attach ) {
		account addft;
		addft.id = st.id;
		addft.submitted_by = submitted_by;
		addft.balance = quantity;

		newcontainerf.push_back( addft );
	}

	if ( !attach )
		check( found, "not attached" );

	assets_f.modify( itr, submitted_by, [&]( auto& a ) {
		a.containerf = newcontainerf;
	});

	if ( attach ) {
		sub_balancef( platform, submitted_by, quantity );
	}
	else {
		add_balancef( platform, submitted_by, quantity, platform );
	}
}

void Assets::sub_balancef( name platform, name submitted_by, asset value ) {

	accounts from_acnts( _self, platform.value );
	const auto& from = from_acnts.get( getFTIndex( submitted_by, value.symbol ), "no balance object found" );
	check( from.balance.amount >= value.amount, "overdrawn balance" );
	check( value.symbol.code().raw() == from.balance.symbol.code().raw(), "Wrong symbol" );

	from_acnts.modify( from, has_auth( submitted_by ) ? submitted_by : platform, [&]( auto& a ) {
		a.balance -= value;
	});
}

void Assets::add_balancef( name platform, name submitted_by, asset value, name ram_payer ) {

	accounts to_acnts( _self, platform.value );
	auto ftid = getFTIndex( submitted_by, value.symbol );
	auto to = to_acnts.find( ftid );

	if ( to == to_acnts.end() ) {
		to_acnts.emplace( ram_payer, [&]( auto& a ) {
			a.id = ftid;
			a.balance = value;
			a.submitted_by = submitted_by;
		});
	}
	else {
		to_acnts.modify( to, same_payer, [&]( auto& a ) {
			a.balance += value;
		});
	}
}

template<typename... Args>
void Assets::sendEvent( name submitted_by, name rampayer, name seaction, const std::tuple<Args...> &adata ) {

	transaction sevent{};
	sevent.actions.emplace_back( permission_level{ _self, "active"_n }, submitted_by, seaction, adata );
	sevent.delay_sec = 0;
	sevent.send( getid("DEFER"), rampayer );
}

asset Assets::get_supply( name token_contract_account, name submitted_by, symbol_code sym_code ) {
	stats statstable( token_contract_account, submitted_by.value );
	return statstable.get( sym_code.raw() ).supply;
}

asset Assets::get_balance( name token_contract_account, name platform, name submitted_by, symbol_code sym_code ) {
	stats statstable( token_contract_account, submitted_by.value );
	accounts accountstable( token_contract_account, platform.value );
	return accountstable.get( statstable.get( sym_code.raw() ).id ).balance;
}

std::vector<string> Assets::splitWord(string& s, char delimiter) {
  std::vector<string> tokens;
  string token;
  size_t pos = 0;
  while ((pos = s.find(delimiter)) != string::npos) {
    token = s.substr(0, pos);
    tokens.push_back(token);
    s.erase(0, pos + 1);
  }
  tokens.push_back(s);
  return tokens;
}

string Assets::join(const std::vector<string> &lst, const string &delim) {
  string ret;
  for (const auto &s : lst) {
    if (!ret.empty())
      ret += delim;
    ret += s;
  }
  return ret;
}

std::vector<std::vector<string>> Assets::groupBy(std::vector<string> digest, int size) {
  std::vector<std::vector<string>> groupDigest;
  for (int i = 0; i < digest.size() / size; i++) {
    std::vector<string> tmp;
    for (int y = 0; y < size; y++) {
      tmp.push_back(digest[(i * size) + y]);
    }
    groupDigest.push_back(tmp);
  }
  return groupDigest;
}

std::vector<std::vector<checksum256>> Assets::getBucket(string& digestString, string& type) {
    std::vector<string> digest = splitWord(digestString, ',');
    std::vector<std::vector<std::vector<string>>> digestGroup;
	std::vector<std::vector<checksum256>> buckets;

	if (type.compare("TEXT") == 0) {
	  digestGroup.push_back(groupBy(digest, 5));
	  digestGroup.push_back(groupBy(digest, 9));
	  digestGroup.push_back(groupBy(digest, 13));
	} else if (type.compare("IMAGE") == 0){
	  digestGroup.push_back(groupBy(digest, 1));
	}
	const int length = digestGroup.size();
	for (int i = 0; i < length; i++) {
	  std::vector<checksum256> smallBuckets;
      for (int j = 0; j < digestGroup[i].size(); j++) {
        string bucket;
        bucket = std::to_string(j) + "_" + join(digestGroup[i][j], "_");
        checksum256 bucket256 = sha256(bucket.c_str(), bucket.size() * sizeof(char));
	    smallBuckets.push_back(bucket256);
      }
	  buckets.push_back(smallBuckets);
	}
	return buckets;
}
std::tuple<bool, std::vector<uint64_t>, std::vector<checksum256> > Assets::checkDuplicate(std::vector<std::vector<checksum256>> buckets, string type) {
	bool isDuplicate = true;
	std::vector<uint64_t> duplicateAssetID; // for logging duplicate with asset ids.
	std::vector<checksum256> digestsForInsert;
	if (type.compare("TEXT") == 0) {
		stextdigests digests_f(_self, _self.value);
	    auto digest_index = digests_f.get_index<name("digest")>();
	    for (int i =0; i < buckets.size(); i++) {
          bool isInnerDuplicate = false; // flag for check duplicate in digest group
	      for (int j = 0; j < buckets[i].size(); j++) {
	      	const auto itr = digest_index.find(buckets[i][j]);
			if(itr != digest_index.end()) {
              isInnerDuplicate = true;
              duplicateAssetID.push_back(itr->asset_id); 
			} else if(itr == digest_index.end()) {
				if (!isInnerDuplicate) isDuplicate = false; //if digest group not duplicate then set flag not dup
			  	digestsForInsert.push_back(buckets[i][j]);
			}
	      }
		}
	} else if (type.compare("IMAGE") == 0) {
		simagedigests digests_f(_self, _self.value);
	    auto digest_index = digests_f.get_index<name("digest")>();
	    for (int i =0; i < buckets.size(); i++) {
          bool isInnerDuplicate = false; // flag for check duplicate in digest group
	      for (int j = 0; j < buckets[i].size(); j++) {
	      	const auto itr = digest_index.find(buckets[i][j]);
			if(itr != digest_index.end()) {
              isInnerDuplicate = true;
              duplicateAssetID.push_back(itr->asset_id); 
			} else if(itr == digest_index.end()) {
				if (!isInnerDuplicate) isDuplicate = false; //if digest group not duplicate then set flag not dup
			  	digestsForInsert.push_back(buckets[i][j]);
			}
	      }
		}
	}
	sort( duplicateAssetID.begin(), duplicateAssetID.end() );
	duplicateAssetID.erase( unique( duplicateAssetID.begin(), duplicateAssetID.end() ), duplicateAssetID.end() );

	return std::make_tuple(isDuplicate, duplicateAssetID, digestsForInsert);
}


EOSIO_DISPATCH( Assets, (newasset)( create )( newassetlog )( createlog )( transfer )( revoke ) 
( offer )( canceloffer )( claim )( setmdata )( setdinfo ) ( updatecinfo )
( regsubmitted )( submittedud )
( delegate )( undelegate )( delegatemore )( attach )( detach )
( createf )( updatef )( issuef )( transferf )( revokef )
( offerf )( cancelofferf )( claimf )
( attachf )( detachf )( openf )( closef )
( updatever ) (cleartables1) (cleartables2) )

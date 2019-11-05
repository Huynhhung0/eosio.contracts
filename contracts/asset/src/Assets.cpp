#include <Assets.hpp>

ACTION Assets::cleartables() {
  require_auth(_self);
  simagedigests st(_self, _self.value);
  auto itr = st.begin();
  while(itr != st.end()) {
    itr = st.erase(itr);
  }
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

ACTION Assets::regcreator( name creator, string data, string stemplate, string imgpriority ) {

	require_auth( creator );
	require_recipient( creator );
	check( data.size() > 3, "Data field is too short. Please tell us about yourselves." );
	creators creator_( _self, _self.value );

	if ( creator_.find( creator.value ) == creator_.end() ) {
		creator_.emplace( creator, [&]( auto& s ) {
			s.creator = creator;
			s.data = data;
			s.stemplate = stemplate;
			s.imgpriority = imgpriority;
		});
	}
	else {
		check( false, "Registration Error. You're probably already registered. Try the authupdate action." );
	}
}

ACTION Assets::creatorupdate( name creator, string data, string stemplate, string imgpriority ) {

	require_auth( creator );
	require_recipient( creator );
	creators creator_( _self, _self.value );
	auto itr = creator_.find( creator.value );
	check( itr != creator_.end(), "creator not registered" );

	if ( data.empty() && stemplate.empty() ) {
		itr = creator_.erase( itr );
	}
	else {
		creator_.modify( itr, creator, [&]( auto& s ) {
			s.data = data;
			s.stemplate = stemplate;
			s.imgpriority = imgpriority;
		});
	}
}

//@TODO dont allow an creator run brute force new assetid.
ACTION Assets::newasset(name creator) {
	require_auth(creator);
	check( is_account( creator ), "creator account does not exist." );
	require_recipient( creator );
	const auto newID = getid();
	sassets assets( _self, creator.value );
	string empty = "";
	bool no = false;

	assets.emplace( _self, [&]( auto& s ) {
		s.id = newID;
		s.owner = creator;
		s.creator = creator;
	});

	//Events
	sendEvent( creator, creator, "saenewasset"_n, std::make_tuple( creator, newID ) );
	SEND_INLINE_ACTION( *this, newassetlog, { {_self, "active"_n} }, { creator, newID} );

}

ACTION Assets::create(uint64_t assetid, name creator, name owner, string idata, string mdata, bool requireclaim ) {

	require_auth( creator );
	sassets assets_f( _self, creator.value );
	const auto itrAsset = assets_f.find( assetid );
	check( itrAsset != assets_f.end(), "asset not found" );
	check( itrAsset->creator == creator, "Only creator can update asset." );
	check( itrAsset->idata.compare("") == 0, "Can not update asset data." );
	check( !( creator.value == owner.value && requireclaim == 1 ), "Can't requireclaim if creator == owner." );
	json js = json::parse(idata);	
	string digestString;
	string type;
	for (auto& [key, value] : js.items()) {
		if (key.compare("digest") == 0) { 
			digestString = value;
		}
		else if (key.compare("type") == 0) { 
			type = value;
		}
	}
    std::vector<string> digest = splitWord(digestString, ',');
    std::vector<std::vector<string>> digestGroup;
	if (type.compare("TEXT") == 0) {
	  digestGroup = groupBy(digest, 13);
	} else if (type.compare("IMAGE") == 0){
	  digestGroup = groupBy(digest, 1);
	}
	int i,j;
	std::vector<checksum256> buckets;
    for (i = 0,j = 0; i < digestGroup.size(); i++, j++) {
      string bucket;
      bucket = std::to_string(j) + "_" + join(digestGroup[i], "_");
      checksum256 bucket256 = sha256(bucket.c_str(), bucket.size() * sizeof(char));
	  buckets.push_back(bucket256);
    }
	if (type.compare("TEXT") == 0) {
		stextdigests digests_f(_self, _self.value);
	    auto digest_index = digests_f.get_index<name("digest")>();
	    for (i =0; i < buckets.size(); i++) {
	    	const auto itr = digest_index.find(buckets[i]);
			if(itr != digest_index.end()) {
              string msg = "found duplicate text digest with assetID:" +  std::to_string(itr->assetid);
	    	  check(false, msg);
			}
	    }
	    for (i =0; i < buckets.size(); i++) {
			digests_f.emplace( _self, [&]( auto& d ) { d.id = getid("TEXT"); d.digest= buckets[i]; d.assetid = assetid;});
	    }
	} else if (type.compare("IMAGE") == 0) {
		simagedigests digests_f(_self, _self.value);
	    auto digest_index = digests_f.get_index<name("digest")>();
	    for (i =0; i < buckets.size(); i++) {
	    	const auto itr = digest_index.find(buckets[i]);
			if(itr != digest_index.end()) {
              string msg = "found duplicate image digest with assetID:" +  std::to_string(itr->assetid);
	    	  check(false, msg);
			}
	    }
	    for (i =0; i < buckets.size(); i++) {
			digests_f.emplace( _self, [&]( auto& d ) { d.id = getid("IMAGE"); d.digest= buckets[i]; d.assetid = assetid;});
	    }

	} else {
		check(false, "invalid type");
	}
	assets_f.modify( itrAsset, creator, [&]( auto& a ) {
		a.owner = owner;
		a.mdata = mdata; // mutable data
		a.idata = idata; // immutable data
	});

	//Events
	sendEvent( creator, creator, "saecreate"_n, std::make_tuple( owner, assetid) );
	SEND_INLINE_ACTION( *this, createlog, { {_self, "active"_n} }, { creator, owner, idata, mdata, assetid, requireclaim } );

}
ACTION Assets::newassetlog( name creator, uint64_t assetid) {

	require_auth(get_self());
}

ACTION Assets::createlog( name creator, name owner, string idata, string mdata, uint64_t assetid, bool requireclaim ) {

	require_auth(get_self());
}

ACTION Assets::claim( name claimer, std::vector<uint64_t>& assetids ) {

	require_auth( claimer );
	require_recipient( claimer );
	offers offert( _self, _self.value );
	sassets assets_t( _self, claimer.value );

	std::map< name, std::map< uint64_t, name > > uniqcreator;
	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itrc = offert.find( assetids[i] );
		check( itrc != offert.end(), "Cannot find at least one of the assets you're attempting to claim." );
		check( claimer == itrc->offeredto, "At least one of the assets has not been offerred to you." );

		sassets assets_f( _self, itrc->owner.value );
		auto itr = assets_f.find( assetids[i] );
		check( itr != assets_f.end(), "Cannot find at least one of the assets you're attempting to claim." );
		check( itrc->owner.value == itr->owner.value, "Owner was changed for at least one of the items!?" );

		assets_t.emplace( claimer, [&]( auto& s ) {
			s.id = itr->id;
			s.owner = claimer;
			s.creator = itr->creator;
			s.mdata = itr->mdata; 		// mutable data
			s.idata = itr->idata; 		// immutable data
			s.container = itr->container;
			s.containerf = itr->containerf;
		});

		//Events
		uniqcreator[itr->creator][assetids[i]] = itrc->owner;

		assets_f.erase(itr);
		offert.erase(itrc);
	}

	for ( auto uniqcreatorIt = uniqcreator.begin(); uniqcreatorIt != uniqcreator.end(); ++uniqcreatorIt ) {
		name keycreator = std::move( uniqcreatorIt->first );
		sendEvent( keycreator, claimer, "saeclaim"_n, std::make_tuple( claimer, uniqcreator[keycreator] ) );
	}
}

ACTION Assets::transfer( name from, name to, string fromjsonstr, string tojsonstr, std::vector<uint64_t>& assetids, string memo ) {

	check( from != to, "cannot transfer to yourself" );
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

	std::map< name, std::vector<uint64_t> > uniqcreator;

	json fromjson = json::parse(fromjsonstr);
	json tojson = json::parse(tojsonstr);

	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itrd = delegatet.find( assetids[i] );
		isDelegeting = false;
		if ( itrd != delegatet.end() ) {
			if ( itrd->owner == to || itrd->delegatedto == to ) {
				isDelegeting = true;
				if ( itrd->owner == to ) {
					delegatet.erase( itrd );
				}
			}
			else {
				check( false, "At least one of the assets cannot be transferred because it is delegated" );
			}
		}

		if ( isDelegeting ) {
			require_auth( has_auth( itrd->owner ) ? itrd->owner : from );
		}
		else {
			require_auth( from );
		}

		auto itr = assets_f.find( assetids[i] );
		json mdata = json::parse(itr->mdata);
		string ownerState = mdata["echo_owner"].get<string>();
		string refOwnerState = mdata["echo_ref_owner"].get<string>();
		string fromOwner = fromjson["echo_owner"].get<string>();
		string fromRefOwner = fromjson["echo_ref_owner"].get<string>();
		string toOwner = tojson["echo_owner"].get<string>();
		string toRefOwner = tojson["echo_ref_owner"].get<string>();
		check(ownerState.compare(fromOwner) == 0, "cannot transfer from other owner.");
		check(refOwnerState.compare(fromRefOwner) == 0, "cannot transfer from other ref owner.");
		check(ownerState.compare(toOwner) != 0, "cannot transfer to yourself.");
		check(refOwnerState.compare(toRefOwner) != 0, "cannot transfer to yourself.");
		check( itr != assets_f.end(), "At least one of the assets cannot be found (check ids?)" );
		check( from.value == itr->owner.value, "At least one of the assets is not yours to transfer." );
		check( offert.find( assetids[i] ) == offert.end(), "At least one of the assets has been offered for a claim and cannot be transferred. Cancel offer?" );
		mdata["echo_owner"] = toOwner;
		mdata["echo_ref_owner"] = toRefOwner;
		assets_t.emplace( rampayer, [&]( auto& s ) {
			s.id = itr->id;
			s.owner = to;
			s.creator = itr->creator;
			s.idata = itr->idata; 		// immutable data
			s.mdata = mdata.dump(); 		   // mutable data
			s.container = itr->container;
			s.containerf = itr->containerf;

		});

		//Events
		uniqcreator[itr->creator].push_back( assetids[i] );
		assets_f.erase(itr);
	}

	//Send Event as deferred
	for ( auto uniqcreatorIt = uniqcreator.begin(); uniqcreatorIt != uniqcreator.end(); ++uniqcreatorIt ) {
		name keycreator = std::move( uniqcreatorIt->first );
		sendEvent( keycreator, rampayer, "saetransfer"_n, std::make_tuple( from, to, uniqcreator[keycreator], memo ) );
	}
}

ACTION Assets::update( name owner, uint64_t assetid, string mdata ) {

	require_auth( owner );
	sassets assets_f( _self, owner.value );
	const auto itr = assets_f.find( assetid );
	check( itr != assets_f.end(), "asset not found" );
	check( itr->owner == owner, "Only owner can update asset." );

	assets_f.modify( itr, owner, [&]( auto& a ) {
		a.mdata = mdata;
	});
}

ACTION Assets::offer( name owner, name newowner, std::vector<uint64_t>& assetids, string memo ) {

	check( owner != newowner, "cannot offer to yourself" );
	require_auth( owner );
	require_recipient( owner );
	require_recipient( newowner );
	check( is_account( newowner ), "newowner account does not exist" );

	sassets assets_f( _self, owner.value );
	offers offert( _self, _self.value );
	delegates delegatet( _self, _self.value );

	for ( auto i = 0; i < assetids.size(); ++i ) {
		check( assets_f.find( assetids[i] ) != assets_f.end(), "At least one of the assets was not found." );
		check( offert.find( assetids[i] ) == offert.end(), "At least one of the assets is already offered for claim." );
		check( delegatet.find( assetids[i] ) == delegatet.end(), "At least one of the assets is delegated and cannot be offered." );

		offert.emplace( owner, [&]( auto& s ) {
			s.assetid = assetids[i];
			s.offeredto = newowner;
			s.owner = owner;
			s.cdate = now();
		});
	}
}

ACTION Assets::canceloffer( name owner, std::vector<uint64_t>& assetids ) {

	require_auth( owner );
	require_recipient( owner );
	offers offert( _self, _self.value );

	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itr = offert.find( assetids[i] );
		check( itr != offert.end(), "The offer for at least one of the assets was not found." );
		check( owner.value == itr->owner.value, "You're not the owner of at least one of the assets whose offers you're attempting to cancel." );
		offert.erase( itr );
	}
}

ACTION Assets::revoke( name owner, std::vector<uint64_t>& assetids, string memo ) {

	require_auth( owner );
	sassets assets_f( _self, owner.value );
	stextdigests tdigests_f(_self, _self.value);
	simagedigests idigests_f(_self, _self.value);
	offers offert( _self, _self.value );
	delegates delegatet( _self, _self.value );

	std::map< name, std::vector<uint64_t> > uniqcreator;

	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itr = assets_f.find( assetids[i] );
		check( itr != assets_f.end(), "At least one of the assets was not found." );
		check( owner.value == itr->owner.value, "At least one of the assets you're attempting to revoke is not yours." );
		check( offert.find( assetids[i] ) == offert.end(), "At least one of the assets has an open offer and cannot be revokeed." );
		check( delegatet.find( assetids[i] ) == delegatet.end(), "At least one of assets is delegated and cannot be revokeed." );

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
		   auto itr = idx.find(assetids[i]);
		   if(itr == idx.end()) {
			   break;
		   }
		   idx.erase(itr);
		  }
	    } else if (type.compare("IMAGE") == 0){
		  while (true){ 
            auto idx = idigests_f.get_index<name("asset")>();
		    auto itr = idx.find(assetids[i]);
		    if(itr == idx.end()) {
		  	  break;
		    }
		    idx.erase(itr);
		  }
	    }

		//Events
		uniqcreator[itr->creator].push_back( assetids[i] );
		assets_f.erase(itr);
	}

	//Send Event as deferred
	for ( auto uniqcreatorIt = uniqcreator.begin(); uniqcreatorIt != uniqcreator.end(); ++uniqcreatorIt ) {
		name keycreator = std::move( uniqcreatorIt->first );
		sendEvent( keycreator, owner, "saerevoke"_n, std::make_tuple( owner, uniqcreator[keycreator], memo ) );
	}
}

ACTION Assets::delegate( name owner, name to,string fromjson, string tojson, std::vector<uint64_t>& assetids, uint64_t period, string memo ) {

	check(memo.size() <= 64, "Error. Size of memo cannot be bigger 64");
	check( owner != to, "cannot delegate to yourself" );
	require_auth( owner );
	require_recipient( owner );
	check( is_account( to ), "TO account does not exist" );

	sassets assets_f( _self, owner.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );

	for ( auto i = 0; i < assetids.size(); ++i ) {
		check( assets_f.find( assetids[i] ) != assets_f.end(), "At least one of the assets cannot be found." );
		check( delegatet.find( assetids[i] ) == delegatet.end(), "At least one of the assets is already delegated." );
		check( offert.find( assetids[i] ) == offert.end(), "At least one of the assets has an open offer and cannot be delegated." );

		delegatet.emplace( owner, [&]( auto& s ) {
			s.assetid = assetids[i];
			s.owner = owner;
			s.delegatedto = to;
			s.cdate = now();
			s.period = period;
			s.memo = memo;
		});
	}

	transfer( owner, to, fromjson, tojson, assetids, "Delegate memo: " + memo );
}

ACTION Assets::delegatemore( name owner, uint64_t assetidc, uint64_t period ) {

	require_auth( owner );
	require_recipient( owner );

	delegates delegatet( _self, _self.value );
	const auto itrc = delegatet.find( assetidc );
	check( itrc != delegatet.end(), "Assets assetidc is not delegated." );
	
	delegatet.modify( itrc, owner, [&]( auto& s ) {
		s.period = itrc->period + period;
	});
}

ACTION Assets::undelegate( name owner, name from,string fromjson, string tojson, std::vector<uint64_t>& assetids ) {

	require_auth( owner );
	require_recipient( owner );
	check( is_account( from ), "to account does not exist" );

	sassets assets_f( _self, from.value );
	delegates delegatet( _self, _self.value );

	string assetidsmemo;
	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itr = assets_f.find( assetids[i] );
		check( itr != assets_f.end(), "At least one of the assets cannot be found." );
		auto itrc = delegatet.find( assetids[i] );
		check( itrc != delegatet.end(), "At least one of the assets is not delegated." );
		check( owner == itrc->owner, "You are not the owner of at least one of these assets." );
		check( from == itrc->delegatedto, "FROM does not match DELEGATEDTO for at least one of the assets." );
		check( itr->owner == itrc->delegatedto, "FROM does not match DELEGATEDTO for at least one of the assets." );
		check( ( itrc->cdate + itrc->period ) < now(), "Cannot undelegate until the PERIOD expires." );

		if ( i != 0 ) {
			assetidsmemo += ", ";
		}

		assetidsmemo += std::to_string( assetids[i] );
	}

	transfer( from, owner, fromjson, tojson, assetids, "undelegate assetid: " + assetidsmemo );
}


ACTION Assets::attach( name owner, uint64_t assetidc, std::vector<uint64_t>& assetids ) {

	sassets assets_f( _self, owner.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );
	require_recipient( owner );
	const auto ac_ = assets_f.find( assetidc );
	check( ac_ != assets_f.end(), "Asset cannot be found." );
	require_auth( ac_->creator );

	for ( auto i = 0; i < assetids.size(); ++i ) {
		auto itr = assets_f.find( assetids[i] );
		check( itr != assets_f.end(), "At least one of the assets cannot be found." );
		check( assetidc != assetids[i], "Cannot attcach to self." );
		check( itr->creator == ac_->creator, "Different creators." );
		check( delegatet.find( assetids[i] ) == delegatet.end(), "At least one of the assets is delegated." );
		check( offert.find( assetids[i] ) == offert.end(), "At least one of the assets has an open offer and cannot be delegated." );

		assets_f.modify( ac_, ac_->creator, [&]( auto& a ) {
			a.container.push_back( *itr );
		});
		assets_f.erase( itr );
	}
}

ACTION Assets::detach( name owner, uint64_t assetidc, std::vector<uint64_t>& assetids ) {

	require_auth( owner );
	require_recipient( owner );
	sassets assets_f( _self, owner.value );

	const auto ac_ = assets_f.find( assetidc );
	check( ac_ != assets_f.end(), "Asset cannot be found." );

	delegates delegatet( _self, _self.value );
	check( delegatet.find( assetidc ) == delegatet.end(), "Cannot detach from delegated. assetidc is delegated." );

	for ( auto i = 0; i < assetids.size(); ++i ) {
		std::vector<sasset> newcontainer;

		for ( auto j = 0; j < ac_->container.size(); ++j ) {
			auto acc = ac_->container[j];
			if ( assetids[i] == acc.id ) {
				assets_f.emplace( owner, [&]( auto& s ) {
					s.id = acc.id;
					s.owner = owner;
					s.creator = acc.creator;
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

		assets_f.modify( ac_, owner, [&]( auto& a ) {
			a.container = newcontainer;
		});
	}
}

ACTION Assets::attachf( name owner, name creator, asset quantity, uint64_t assetidc ) {

	attachdeatch( owner, creator, quantity, assetidc, true );
}


ACTION Assets::detachf( name owner, name creator, asset quantity, uint64_t assetidc ) {

	attachdeatch( owner, creator, quantity, assetidc, false );
}

ACTION Assets::createf( name creator, asset maximum_supply, bool creatorctrl, string data ) {

	require_auth( creator );
	const auto sym = maximum_supply.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( maximum_supply.is_valid(), "invalid supply" );
	check( maximum_supply.amount > 0, "max-supply must be positive" );

	stats statstable( _self, creator.value );
	check( statstable.find( sym.code().raw() ) == statstable.end(), "token with symbol already exists" );

	statstable.emplace( creator, [&]( auto& s ) {
		s.supply.symbol = maximum_supply.symbol;
		s.max_supply = maximum_supply;
		s.issuer = creator;
		s.id = getid();
		s.creatorctrl = creatorctrl;
		s.data = data;
	});
}

ACTION Assets::updatef( name creator, symbol sym, string data ) {

	require_auth( creator );
	check( sym.is_valid(), "invalid symbol name" );
	stats statstable( _self, creator.value );
	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "Symbol not exists" );

	statstable.modify( existing, creator, [&]( auto& a ) {
		a.data = data;
	});
}

ACTION Assets::issuef( name to, name creator, asset quantity, string memo ) {

	const auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	stats statstable( _self, creator.value );
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

	add_balancef( existing->issuer, creator, quantity, existing->issuer );

	if ( to != existing->issuer ) {
		transferf( existing->issuer, to, creator, quantity, memo );
	}
}

ACTION Assets::transferf( name from, name to, name creator, asset quantity, string memo ) {

	check( from != to, "cannot transfer to self" );
	check( is_account( to ), "to account does not exist" );
	const auto sym = quantity.symbol.code();
	stats statstable( _self, creator.value );
	const auto& st = statstable.get( sym.raw() );

	require_recipient( from );
	require_recipient( to );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must transfer positive quantity" );
	check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	auto payer = has_auth( to ) ? to : from;
	auto checkAuth = from;

	if ( st.creatorctrl && has_auth( st.issuer ) ) {
		checkAuth = st.issuer;
		payer = st.issuer;
	}

	require_auth( checkAuth );
	sub_balancef( from, creator, quantity );
	add_balancef( to, creator, quantity, payer );
}

ACTION Assets::offerf( name owner, name newowner, name creator, asset quantity, string memo ) {

	require_auth( owner );
	require_recipient( owner );
	require_recipient( newowner );
	check( is_account( newowner ), "newowner account does not exist" );
	check( owner != newowner, "cannot offer to yourself" );
	const auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );

	stats statstable( _self, creator.value );
	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist" );
	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must retire positive quantity" );
	check( quantity.symbol == existing->supply.symbol, "symbol precision mismatch" );

	offerfs offert( _self, _self.value );
	auto owner_index = offert.template get_index< "owner"_n >();

	for ( auto itro = owner_index.find( owner.value ); itro != owner_index.end(); itro++ ) {
		check( !( itro->creator == creator && itro->offeredto == newowner && itro->quantity.symbol == quantity.symbol ), "Such an offer already exists" );
	}

	offert.emplace( owner, [&]( auto& s ) {
		s.id = getid("DEFER");
		s.creator = creator;
		s.quantity = quantity;
		s.offeredto = newowner;
		s.owner = owner;
		s.cdate = now();
	});
	sub_balancef( owner, creator, quantity );
}

ACTION Assets::cancelofferf( name owner, std::vector<uint64_t>& ftofferids ) {

	require_auth( owner );
	require_recipient( owner );
	offerfs offert( _self, _self.value );

	for ( auto i = 0; i < ftofferids.size(); ++i ) {
		auto itr = offert.find( ftofferids[i] );
		check( itr != offert.end(), "The offer for at least one of the FT was not found." );
		check( owner.value == itr->owner.value, "You're not the owner of at least one of those FTs." );
		add_balancef( owner, itr->creator, itr->quantity, owner );
		offert.erase( itr );
	}
}

ACTION Assets::claimf( name claimer, std::vector<uint64_t>& ftofferids ) {

	require_auth( claimer );
	require_recipient( claimer );
	offerfs offert( _self, _self.value );
	std::map< name, std::vector< uint64_t > > uniqcreator;

	for ( auto i = 0; i < ftofferids.size(); ++i ) {
		auto itrc = offert.find( ftofferids[i] );
		check( itrc != offert.end(), "Cannot find at least one of the FT you're attempting to claim." );
		check( claimer == itrc->offeredto, "At least one of the FTs has not been offerred to you." );
		add_balancef( claimer, itrc->creator, itrc->quantity, claimer );
		offert.erase( itrc );
	}
}

ACTION Assets::revokef( name from, name creator, asset quantity, string memo ) {

	auto sym = quantity.symbol;
	check( sym.is_valid(), "invalid symbol name" );
	check( memo.size() <= 256, "memo has more than 256 bytes" );
	stats statstable( _self, creator.value );

	const auto existing = statstable.find( sym.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist" );
	require_auth( existing->creatorctrl && has_auth( existing->issuer ) ? existing->issuer : from );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must retire positive quantity" );
	check( quantity.symbol == existing->supply.symbol, "symbol precision mismatch" );

	statstable.modify( *existing, same_payer, [&]( auto& s ) {
		s.supply -= quantity;
	});

	sub_balancef( from, creator, quantity );
}

ACTION Assets::openf( name owner, name creator, const symbol& symbol, name ram_payer ) {

	require_auth( ram_payer );
	stats statstable( _self, creator.value );
	const auto& st = statstable.get( symbol.code().raw(), "symbol does not exist" );
	check( st.supply.symbol == symbol, "symbol precision mismatch" );
	accounts acnts( _self, owner.value );

	if ( acnts.find( st.id ) == acnts.end() ) {
		acnts.emplace( ram_payer, [&]( auto& a ) {
			a.id = st.id;
			a.creator = creator;
			a.balance = asset{ 0, symbol };
		});
	}
}

ACTION Assets::closef( name owner, name creator, const symbol& symbol ) {

	require_auth( owner );
	accounts acnts( _self, owner.value );
	auto it = acnts.find( getFTIndex( creator, symbol ) );
	check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
	check( it->balance.amount == 0, "Cannot close because the balance is not zero." );

	offerfs offert( _self, _self.value );
	const auto owner_index = offert.template get_index< "owner"_n >();
	for ( auto itro = owner_index.find( owner.value ); itro != owner_index.end(); itro++ ) {
		check( !( itro->creator == creator && itro->quantity.symbol == symbol ), "You have open offers for this FT.." );
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

uint64_t Assets::getFTIndex( name creator, symbol symbol ) {

	stats statstable( _self, creator.value );
	const auto existing = statstable.find( symbol.code().raw() );
	check( existing != statstable.end(), "token with symbol does not exist." );
	return existing->id;
}

void Assets::attachdeatch( name owner, name creator, asset quantity, uint64_t assetidc, bool attach ) {

	sassets assets_f( _self, owner.value );
	delegates delegatet( _self, _self.value );
	offers offert( _self, _self.value );
	stats statstable( _self, creator.value );
	const auto& st = statstable.get( quantity.symbol.code().raw() );

	require_recipient( owner );

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must transfer positive quantity" );
	check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
	check( st.issuer == creator, "Different creators" );

	if ( attach ) {
		require_auth( creator );	 //attach
	}
	else {
		require_auth( owner );  //deatach
	}

	const auto itr = assets_f.find( assetidc );
	check( itr != assets_f.end(), "assetid cannot be found." );
	check( itr->creator == creator, "Different creators." );
	check( delegatet.find(assetidc) == delegatet.end(), "Asset is delegated." );
	check( offert.find(assetidc) == offert.end(), "Assets has an open offer and cannot be delegated." );

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
		addft.creator = creator;
		addft.balance = quantity;

		newcontainerf.push_back( addft );
	}

	if ( !attach )
		check( found, "not attached" );

	assets_f.modify( itr, creator, [&]( auto& a ) {
		a.containerf = newcontainerf;
	});

	if ( attach ) {
		sub_balancef( owner, creator, quantity );
	}
	else {
		add_balancef( owner, creator, quantity, owner );
	}
}

void Assets::sub_balancef( name owner, name creator, asset value ) {

	accounts from_acnts( _self, owner.value );
	const auto& from = from_acnts.get( getFTIndex( creator, value.symbol ), "no balance object found" );
	check( from.balance.amount >= value.amount, "overdrawn balance" );
	check( value.symbol.code().raw() == from.balance.symbol.code().raw(), "Wrong symbol" );

	from_acnts.modify( from, has_auth( creator ) ? creator : owner, [&]( auto& a ) {
		a.balance -= value;
	});
}

void Assets::add_balancef( name owner, name creator, asset value, name ram_payer ) {

	accounts to_acnts( _self, owner.value );
	auto ftid = getFTIndex( creator, value.symbol );
	auto to = to_acnts.find( ftid );

	if ( to == to_acnts.end() ) {
		to_acnts.emplace( ram_payer, [&]( auto& a ) {
			a.id = ftid;
			a.balance = value;
			a.creator = creator;
		});
	}
	else {
		to_acnts.modify( to, same_payer, [&]( auto& a ) {
			a.balance += value;
		});
	}
}

template<typename... Args>
void Assets::sendEvent( name creator, name rampayer, name seaction, const std::tuple<Args...> &adata ) {

	transaction sevent{};
	sevent.actions.emplace_back( permission_level{ _self, "active"_n }, creator, seaction, adata );
	sevent.delay_sec = 0;
	sevent.send( getid("DEFER"), rampayer );
}

asset Assets::get_supply( name token_contract_account, name creator, symbol_code sym_code ) {
	stats statstable( token_contract_account, creator.value );
	return statstable.get( sym_code.raw() ).supply;
}

asset Assets::get_balance( name token_contract_account, name owner, name creator, symbol_code sym_code ) {
	stats statstable( token_contract_account, creator.value );
	accounts accountstable( token_contract_account, owner.value );
	return accountstable.get( statstable.get( sym_code.raw() ).id ).balance;
}

std::vector<string> Assets::splitWord(string s, char delimiter) {
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

EOSIO_DISPATCH( Assets, (newasset)( create )( newassetlog )( createlog )( transfer )( revoke )( update )
( offer )( canceloffer )( claim )
( regcreator )( creatorupdate )
( delegate )( undelegate )( delegatemore )( attach )( detach )
( createf )( updatef )( issuef )( transferf )( revokef )
( offerf )( cancelofferf )( claimf )
( attachf )( detachf )( openf )( closef )
( updatever ) (cleartables) )

/*

 * @version 1.1.0
 *
 * @section LICENSE
 *
 * This program is under GNU LESSER GENERAL PUBLIC LICENSE.
 * Version 2.1, February 1999.
 * The licenses for most software are designed to take away your
 * freedom to share and change it.  By contrast, the GNU General Public
 * Licenses are intended to guarantee your freedom to share and change
 * free software--to make sure the software is free for all its users.
 * GNU LESSER GENERAL PUBLIC LICENSE for more details at
 * https://github.com/CryptoLions/Assets/blob/master/LICENSE
 *
 * @section DESCRIPTION
 * Assets (Digital Assets)
 *
 * A simple standard for digital assets (ie. Fungible and Non-Fungible Tokens - NFTs) for EOSIO blockchains
 *    WebSite:        https://simpleassets.io
 *    GitHub:         https://github.com/CryptoLions/Assets
 *    Presentation:   https://medium.com/@cryptolions/introducing-simple-assets-b4e17caafaa4
 *    Event Receiver: https://github.com/CryptoLions/Assets-EventReceiverExample
 */

#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include <nlohmann/json.hpp>


using namespace eosio;
using json = nlohmann::json;
using std::string;

uint32_t now() {
    return (uint32_t)(current_time_point().sec_since_epoch());
}

CONTRACT Assets : public contract{
	public:
		using contract::contract;

		/*
		* clear table data.
		*
		* This action clear table data for development only.
		*
		* @return no return value.
		*/

	/*
		ACTION cleartables1();
		using cleartables1_action = action_wrapper< "cleartables1"_n, &Assets::cleartables1 >;
		ACTION cleartables2();
		using cleartables2_action = action_wrapper< "cleartables2"_n, &Assets::cleartables2 >;
	*/

		/*
		* Update version.
		*
		* This action update version of this Assets deployment for 3rd party wallets, marketplaces, etc.
		*
		* @param version is version number of Assetst deployment.
		* @return no return value.
		*/
		ACTION updatever( string version );
		using updatever_action = action_wrapper< "updatever"_n, &Assets::updatever >;

		/*
		* New Creator registration.
		*
		* This action register new submitted_by registration. Action is not mandatory.  Markets *may* choose to use information here
		* to display info about the submitted_by, and to follow specifications expressed here for displaying
		* asset fields.
		*
		* @param submitted_by is submitted_bys account who will create assets.
		* @param data is stringified json. Recommendations to include: game, company, logo, url, desc.
		* @param stemplate	is stringified json with key:state values, where key is key from mdata or idata and
		*		 state indicates recommended way of displaying field:
		* txt		- default
		* url		- show as clickable URL
		* img		- link to img file
		* webgl		- link to webgl file
		* mp3		- link to mp3 file
		* video		- link to video file
		* hide		- do not show
		* imgb 		- image as string in binary format
		* webglb	- webgl binary
		* mp3b 		- mp3 binary
		* videob 	- video binary
		*
		* @param imgpriority is json which assosiates an NFT category with the field name from idata or mdata
		* that specifies the main image 
		*
		* @return no return value
		*/
		ACTION regsubmitted( name submitted_by, string data, string stemplate, string imgpriority );
		using regsubmitted_action = action_wrapper< "regsubmitted"_n, &Assets::regsubmitted >;

		/*
		* Creators info update.
		*
		* This action update submitted_bys info. Used to updated submitted_by information, and asset display recommendations created
		* with the regsubmitted action. This action replaces the fields data and stemplate.
		* To remove submitted_by entry, call this action with null strings for data and stemplate.
		*
		* (See regsubmitted action for parameter info.)
		*
		* @return no return value.
		*/
		ACTION submittedud( name submitted_by, string data, string stemplate, string imgpriority );
		using submittedud_action = action_wrapper< "submittedud"_n, &Assets::submittedud >;

		/*
		* Prepare a new asset.
		*
		* This action Prepare a new asset.
		*
		* @param submitted_by is assets submitted_by.
		* @return no return value.
		*/

		ACTION newasset(name submitted_by, uint64_t num);
		using newasset_action = action_wrapper< "newasset"_n, &Assets::newasset >;

		/*
		ACTION genid(name submitted_by,string r);
		using genid_action = action_wrapper< "genid"_n, &Assets::genid >;
		*/

		/*
		* Create a newasset log.
		*
		* This is empty action. Used by create action to log asset_id so that third party explorers can
		* easily get new asset ids and other information.
		*
		* @param submitted_by is asset's submitted_by, who will able to updated asset's mdata.
		* @param asset_id is id of the asset
		* @return no return value.
		*/
		ACTION newassetlog( name submitted_by, uint64_t asset_id);
		using newassetlog_action = action_wrapper< "newassetlog"_n, &Assets::newassetlog >;

		/*
		* check duplicate digest 
		*
		* This is action. Used by check that digest is duplicate with assets id in table 
		*
		* @param idata is json string.
		* @return no return value.
		*/
		ACTION isduplicate(string idata); 
		using isduplicate_action = action_wrapper< "isduplicate"_n, &Assets::isduplicate >; 

		ACTION isduplog(string duplicate,string duplicateAssetID ); 
		using isduplog_action = action_wrapper< "isduplog"_n, &Assets::isduplog >; 

		/*
		* Create a new asset.
		*
		* This action create a new asset.
		*
		* @param submitted_by is asset's submitted_by, who will able to updated asset's mdata.
		* @param asset_id is asset_id to create.
		* @param platform is assets platform.
		* @param idata is stringified json or just sha256 string with immutable assets data
		* @param mdata is stringified json or just sha256 string with mutable assets data, can be changed only by submitted_by;
		* @param requireclaim is true or false. If disabled, upon creation, the asset will be transfered to platform (but
		*		  but AUTHOR'S memory will be used until the asset is transferred again).  If enabled,
		*		  submitted_by will remain the platform, but an offer will be created for the account specified in
		*		  the platform field to claim the asset using the account's RAM.
		* @return no return value.
		*/
		ACTION create(name submitted_by, uint64_t asset_id,  string idata, string mdata, string common_info, string detail_info, string ref_info );
		using create_action = action_wrapper< "create"_n, &Assets::create >;

		/*
		* Insert Digest.
		*
		* This is empty action. Used by create action to log asset_id so that third party explorers can
		* easily get new asset ids and other information.
		*
		* @param submitted_by	is asset's submitted_by, who will able to updated asset's mdata.
		* @param asset_id is assets id for insert digest.
		* @param idata is stringified json or just sha256 string with immutable assets data.
		* @return no return value.
		*/
		ACTION insertdigest(name submitted_by, uint64_t asset_id, string idata);
		using insertdigest_action = action_wrapper<"insertdigest"_n, &Assets::insertdigest >;

		/*
		* Create a new log.
		*
		* This is empty action. Used by create action to log asset_id so that third party explorers can
		* easily get new asset ids and other information.
		*
		* @param submitted_by	is asset's submitted_by, who will able to updated asset's mdata.
		* @param platform is assets platform.
		* @param idata is stringified json or just sha256 string with immutable assets data.
		* @param mdata is stringified json or just sha256 string with mutable assets data, can be changed only by submitted_by.
		* @param asset_id is id of the asset
		* @param requireclaim is true or false. If disabled, upon creation, the asset will be transfered to platform (but
		*		 but AUTHOR'S memory will be used until the asset is transferred again).  If enabled,
		*		 submitted_by will remain the platform, but an offer will be created for the account specified in
		*		 the platform field to claim the asset using the account's RAM.
		* @return no return value.
		*/
		ACTION createlog( name submitted_by, name platform, string idata, string mdata, string commoninfo, string detailinfo, string refinfo, uint64_t asset_id );
		using createlog_action = action_wrapper< "createlog"_n, &Assets::createlog >;

		/*
		* Create a new digest log.
		*
		* This is empty action. Used by re-work digest action to log asset_id so that third party explorers can
		* easily get new asset ids and other information.
		*
		* @param submitted_by	is asset's submitted_by, who will able to updated asset's mdata.
		* @param asset_id is id of the asset
		* @param idata is stringified json or just sha256 string with immutable assets data.
		* @return no return value.
		*/
		ACTION digestlog( name submitted_by,uint64_t asset_id, string idata );
		using digestlog_action = action_wrapper< "digestlog"_n, &Assets::digestlog >;

		/*
		* Claim asset.
		*
		* This action claime the specified asset (assuming it was offered to claimer by the asset platform).
		*
		* @param claimer is account claiming the asset.
		* @param asset_ids is array of asset_id's to claim.
		* @return no return value.
		*/
		ACTION claim( name claimer, std::vector< uint64_t >& asset_ids );
		using claim_action = action_wrapper< "claim"_n, &Assets::claim >;

		/*
		* Transfers an asset.
		*
		* This action transfer an assets. On transfer platform asset's and scope asset's changes to {{to}}'s.
		* Senders RAM will be charged to transfer asset.
		* Transfer will fail if asset is offered for claim or is delegated.
		*
		* @param from is account who sends the asset.
		* @param to is account of receiver.
		* @param fromjsonstr is json string object contains echo_platform and echo_ref_platform who sends the asset.
		* @param tojsonstr is json string object contains echo_platform and echo_ref_platform of account receiver.
		* @param asset_id is asset_id's to transfer.
		* @param memo is transfers comment.
		* @return no return value. 
		*/
		ACTION transfer( name from, name to,string fromjsonstr, string tojsonstr, uint64_t asset_id, string memo );
		using transfer_action = action_wrapper< "transfer"_n, &Assets::transfer >;

		/*
		* Update assets common info.
		*
		* This action update assets mutable data (mdata) field. Action is available only for submitted_bys.
		*
		* @param platform is current assets platform.
		* @param asset_id is asset_id to update.
		* @param common_info is stringified json with mutable assets data. merge old data with new data.
		* @return no return value.
		*/
		ACTION updatecinfo(name platform, uint64_t asset_id, string common_info );
		using updatecinfo_action = action_wrapper< "updatecinfo"_n, &Assets::updatecinfo >;

		/*
		* Set assets mdata.
		*
		* This action update assets mutable data (mdata) field. Action is available only for submitted_bys.
		*
		* @param platform is current assets platform.
		* @param asset_id is asset_id to update.
		* @param mdata is stringified json with mutable assets data. All mdata will be replaced.
		* @return no return value.
		*/
		ACTION setmdata(name platform, uint64_t asset_id, string mdata );
		using setmdata_action = action_wrapper< "setmdata"_n, &Assets::setmdata >;

		/*
		* Set assets detail info.
		*
		* This action update assets mutable data (mdata) field. Action is available only for submitted_bys.
		*
		* @param platform is current assets platform.
		* @param asset_id is asset_id to update.
		* @param detail_info is stringified json with detail info. All detail_info will be replaced.
		* @return no return value.
		*/
		ACTION setdinfo(name platform, uint64_t asset_id, string detail_info);
		using setdinfo_action = action_wrapper< "setdinfo"_n, &Assets::setdinfo >;

		/*
		* Offer asset for claim.
		*
		* This is an alternative to the transfer action. Offer can be used by an
		* asset platform to transfer the asset without using their RAM. After an offer is made, the account
		* specified in {{newplatform}} is able to make a claim, and take control of the asset using their RAM.
		* Offer action is not available if an asste is delegated (borrowed).
		*
		* @param platform is current asset platform account.
		* @param newplatform is new asset platform, who will able to claim.
		* @param asset_ids is array of asset_id's to offer.
		* @param memo is memo for offer action.
		* @return no return value.
		*/
		ACTION offer( name platform, name newplatform, std::vector< uint64_t >& asset_ids, string memo );
		using offer_action = action_wrapper< "offer"_n, &Assets::offer >;

		/*
		* Cancel offer.
		*
		* This action cancel and remove offer. Available for the asset platform.
		*
		* @param platform		- current asset platform account.
		* @param asset_ids	- array of asset_id's to cancel from offer.
		* @return no return value.
		*/
		ACTION canceloffer( name platform, std::vector<uint64_t>& asset_ids );
		using canceloffer_action = action_wrapper< "canceloffer"_n, &Assets::canceloffer >;

		/*
		* Burn asset.
		*
		* This action revoke asset {{asset_id}}. This action is only available for the asset platform. After executing, the
		* asset will disappear forever, and RAM used for asset will be released.
		*
		* @param platform is current asset platform account.
		* @param asset_id is asset_id's to revoke.
		* @param memo is memo for revoke action.
		* @return no return value.
		*/
		ACTION revoke( name platform, uint64_t asset_id, string memo );
		using revoke_action = action_wrapper< "revoke"_n, &Assets::revoke >;

		/*
		* Burn asset.
		*
		* This action delegates asset to {{to}}. This action changes the asset platform by calling the transfer action.
		* It also adds a record in the delegates table to record the asset as borrowed.  This blocks
		* the asset from all platform actions (transfers, offers, revokeing by borrower).
		*
		* @param platform is current asset platform account.
		* @param to is borrower account name.
		* @param fromjson is current platform of json.
		* @param tojson is to borrower account.
		* @param asset_id is array of asset_id's to delegate.
		* @param period	is time in seconds that the asset will be lent. Lender cannot undelegate until
		*		 the period expires, however the receiver can transfer back at any time.
		* @param memo is memo for delegate action.
		* @return no return value.
		*/
		ACTION delegate( name platform, name to, string fromjson, string tojson, uint64_t asset_id, uint64_t period, string memo );
		using delegate_action = action_wrapper< "delegate"_n, &Assets::delegate >;

		/*
		* Undelegates an asset.
		*
		* This action undelegate an asset from {{from}} account. Executing action by real platform will return asset immediately,
		* and the entry in the delegates table recording the borrowing will be erased.
		*
		* @param platform is real asset platform account.
		* @param from is current account platform (borrower).
		* @param fromjson is current platform of json.
		* @param tojson is to borrower account.
		* @param asset_id is asset_id's to undelegate.
		* @return no return value.
		*/
		ACTION undelegate( name platform, name from, string fromjson, string tojson, uint64_t asset_id );
		using undelegate_action = action_wrapper< "undelegate"_n, &Assets::undelegate >;

		/*
		* Attach non-fungible token.
		*
		* This action attach other NFTs to the specified NFT. Restrictions:
		* 1. Only the Asset Creator can do this
		* 2. All assets must have the same submitted_by
		* 3. All assets much have the same platform
		*
		* @param platform is platform of NFTs.
		* @param asset_idc is id of container NFT.
		* @param asset_ids is array of asset ids to attach.
		* @return no return value.
		*/
		ACTION attach( name platform, uint64_t asset_idc, std::vector< uint64_t >& asset_ids );
		using attach_action = action_wrapper< "attach"_n, &Assets::attach >;

		/*
		* Detach detach non-fungible token.
		*
		* This action detach NFTs from the specified NFT.
		*
		* @param platform is platform of NFTs.
		* @param asset_idc is the id of the NFT from which we are detaching.
		* @param asset_ids is the ids of the NFTS to be detached.
		* @return no return value.
		*/
		ACTION detach( name platform, uint64_t asset_idc, std::vector< uint64_t >& asset_ids );
		using detach_t_action = action_wrapper< "detach"_n, &Assets::detach >;

		/*
		* Extend period of delegated asset.
		*
		* This action extend period of delegated asset.
		*
		* @param platform is platform of NFTs.
		* @param asset_idc is the id of the NFT for which we are extending period.
		* @param period is new added to existing amount of period.
		* @return no return value.
		*/
		ACTION delegatemore( name platform, uint64_t asset_idc, uint64_t period );
		using delegatemore_action = action_wrapper< "delegatemore"_n, &Assets::delegatemore >;

		/*
		* Attach fungible token.
		*
		* This action attach FTs to the specified NFT. Restrictions:
		* 1. Only the Asset Creator can do this
		* 2. All assets must have the same submitted_by
		* 3. All assets much have the same platform
		*
		* @param platform is platform of assets.
		* @param submitted_by	is submitted_by of the assets.
		* @param asset_idc is id of container NFT.
		* @param quantity is quantity to attach and token name (for example: "10 WOOD", "42.00 GOLD").
		* @return no return value.
		*/
		ACTION attachf( name platform, name submitted_by, asset quantity, uint64_t asset_idc );
		using attachf_t_action = action_wrapper< "attachf"_n, &Assets::attachf >;

		/*
		* Detach fungible token or tokens.
		*
		* This action detach FTs from the specified NFT.
		*
		* @param platform is platform of NFTs.
		* @param submitted_by	is submitted_by of the assets.
		* @param asset_idc is id of the container NFT.
		* @param quantity is quantity to detach and token name (for example: 10 WOOD, 42.00 GOLD).
		* @return no return value.
		*/
		ACTION detachf( name platform, name submitted_by, asset quantity, uint64_t asset_idc );
		using detachf_t_action = action_wrapper< "detachf"_n, &Assets::detachf >;

		/*
		* Creates fungible token.
		*
		* This action create fungible token with specified maximum supply; You can not change anything after creation.
		*
		* @param submitted_by is fungible token submitted_by;
		* @param maximum_supply is maximum token supply, example "10000000.0000 GOLD", "10000000 SEED", "100000000.00 WOOD".
		*        Precision is also important here.
		* @param submitted_byctrl is IMPORTANT! If true(1) allows token submitted_by (and not just platform) to revokef and transferf.
		*        Cannot be changed after creation!
		* @param data is stringify json (recommend including keys `img` and `name` for better displaying by markets).
		* @return no return value.
		*/
		ACTION createf( name submitted_by, asset maximum_supply, bool submitted_byctrl, string data );
		using createf_action = action_wrapper< "createf"_n, &Assets::createf >;

		/*
		* Update fungible token.
		*
		* Update the data field of a fungible token.
		*
		* @param submitted_by is fungible token submitted_by.
		* @param sym is fingible token symbol ("GOLD", "WOOD", etc.).
		* @param data is stringify json (recommend including keys `img` and `name` for better displaying by markets).
		* @return no return value.
		*/
		ACTION updatef( name submitted_by, symbol sym, string data );
		using updatef_action = action_wrapper< "updatef"_n, &Assets::updatef >;

		/*
		* Issue fungible token.
		*
		* This action issues a fungible token.
		*
		* @param to is account receiver.
		* @param submitted_by is fungible token submitted_by.
		* @param quantity is amount to issue, example "1000.00 WOOD".
		* @param memo is issue comment.
		* @return no return value.
		*/
		ACTION issuef( name to, name submitted_by, asset quantity, string memo );
		using issuef_action = action_wrapper< "issuef"_n, &Assets::issuef >;

		/*
		* Transfer fungible token.
		*
		* This action transfer a specified quantity of fungible tokens.
		*
		* @param from is account who sends the token.
		* @param to is account of receiver.
		* @param submitted_by is account of fungible token submitted_by.
		* @param quantity is amount to transfer, example "1.00 WOOD".
		* @param memo is transfer's comment.
		* @return no return value.
		*/
		ACTION transferf( name from, name to, name submitted_by, asset quantity, string memo );
		using transferf_action = action_wrapper< "transferf"_n, &Assets::transferf >;

		/*
		* Offer fungible tokens.
		*
		* This action offer fungible tokens for another EOS user to claim.
		* This is an alternative to the transfer action. Offer can be used by a
		* FT platform to transfer the FTs without using their RAM. After an offer is made, the account
		* specified in {{newplatform}} is able to make a claim, and take control of the asset using their RAM.
		* The FTs will be removed from the platform's balance while the offer is open.
		*
		* @param platform is original platform of the FT.
		* @param newplatform is account which will be able to claim the offer.
		* @param submitted_by is account of fungible token submitted_by.
		* @param quantity is amount to transfer, example "1.00 WOOD".
		* @param memo is offer's comment;
		* @return no return value.
		*/
		ACTION offerf( name platform, name newplatform, name submitted_by, asset quantity, string memo );
		using offerf_action = action_wrapper< "offerf"_n, &Assets::offerf >;

		/*
		* Cancel offer of fungible tokens.
		*
		* This action cancels offer of FTs.
		*
		* @param platform is riginal platform of the FT.
		* @param ftofferids is id of the FT offer.
		* @return no return value.
		*/
		ACTION cancelofferf( name platform, std::vector< uint64_t >& ftofferids );
		using cancelofferf_action = action_wrapper< "cancelofferf"_n, &Assets::cancelofferf >;

		/*
		* Claim fungible tokens.
		*
		* This action claim FTs which have been offered.
		*
		* @param claimer is account claiming FTs which have been offered.
		* @param ftofferids is array of FT offer ids.
		* @return no return value.
		*/
		ACTION claimf( name claimer, std::vector< uint64_t >& ftofferids );
		using claimf_action = action_wrapper< "claimf"_n, &Assets::claimf >;

		/*
		* Burn fungible tokens
		*
		* This action revoke a fungible token. This action is available for the token platform and submitted_by. After executing,
		* accounts balance and supply in stats table for this token will reduce by the specified quantity.
		*
		* @param from is account who revokes the token.
		* @param submitted_by is account of fungible token submitted_by.
		* @param quantity is amount to revoke, example "1.00 WOOD".
		* @param memo is memo for revokef action.
		* @return no return value.
		*/
		ACTION revokef( name from, name submitted_by, asset quantity, string memo );
		using revokef_action = action_wrapper< "revokef"_n, &Assets::revokef >;

		/*
		* Open accoutns table.
		*
		* This action opens accounts table for specified fungible token.
		*
		* @param platform is account where create table with fungible token.
		* @param submitted_by is account of fungible token submitted_by.
		* @param symbol is token symbol, example "WOOD", "ROCK", "GOLD".
		* @param ram_payer is account who will pay for ram used for table creation.
		* @return no return value.
		*/
		ACTION openf( name platform, name submitted_by, const symbol& symbol, name ram_payer );
		using openf_action = action_wrapper< "openf"_n, &Assets::openf >;

		/*
		* Close accounts table.
		*
		* This action close accounts table for provided fungible token and releases RAM.
		* Action works only if balance is 0.
		*
		* @param platform is account who woud like to close table with fungible token.
		* @param submitted_by is account of fungible token submitted_by.
		* @param symbol is token symbol, example "WOOD", "ROCK", "GOLD".
		* @return no return value.
		*/
		ACTION closef( name platform, name submitted_by, const symbol& symbol );
		using closef_action = action_wrapper< "closef"_n, &Assets::closef >;

		/*
		* Return current token supply.
		*
		* This function return current token supply.
		*
		* @param token_contract_account is contract to check.
		* @param submitted_by is fungible tokens submitted_by account.
		* @param sym_code is token symbol, example "WOOD", "ROCK", "GOLD".
		* @return asset
		*/
		static asset get_supply( name token_contract_account, name submitted_by, symbol_code sym_code );

		/*
		* Returns token balance for account.
		*
		* This function return token balance for account.
		*
		* @param token_contract_account is contract to check;
		* @param platform is token holder account;
		* @param submitted_by is fungible tokens submitted_by account;
		* @param sym_code is token symbol, example "WOOD", "ROCK", "GOLD";
		* @return asset
		*/
		static asset get_balance( name token_contract_account, name platform, name submitted_by, symbol_code sym_code );

	private:
		/*
		* Get new asset id.
		*
		* This function return new asset id.
		*
		* @param defer is flag for type of transaction true for defered;
		* @return new asset id
		*/
		uint64_t getid( string type = "ASSET" );
		uint64_t getid( uint64_t );
		uint64_t getiddigest( uint64_t, uint64_t );

		/*
		* Get fungible token index.
		*
		* This function return fungible token index.
		*
		* @param submitted_by is submitted_by name ;
		* @param symbol is symbol;
		* @return new fungible token index
		*/
		uint64_t getFTIndex( name submitted_by, symbol symbol );
		void attachdeatch( name platform, name submitted_by, asset quantity, uint64_t asset_idc, bool attach );
		void sub_balancef( name platform, name submitted_by, asset value );
		void add_balancef( name platform, name submitted_by, asset value, name ram_payer );

		template<typename... Args>
		void sendEvent( name submitted_by, name rampayer, name seaction, const std::tuple<Args...> &tup );
		std::vector<string> splitWord(string& s, char delimiter); 
		string join(const std::vector<string> &lst, const string &delim); 
		std::vector<std::vector<string>> groupBy(std::vector<string> digest, int size);
		std::vector<std::vector<checksum256>> getBucket(string&, string&); 
        std::tuple<bool, std::vector<uint64_t>> checkDuplicate(std::vector<std::vector<checksum256>>, string);

		/*
		* Creators table. Can be used by asset markets, asset explorers, or wallets for correct asset
		* data presentation.
		* Scope: self
		*/
		TABLE ssubmitted_by {
			name			submitted_by;
			string			data;
			string			stemplate;
			string			imgpriority;

			auto primary_key() const {
				return submitted_by.value;
			}

		};
		typedef eosio::multi_index< "submitteds"_n, ssubmitted_by > submitted_bys;

		/*
		* Fungible token accounts stats info: Max Supply, Current Supply, issuer (submitted_by), token unique id, submitted_byctrl.
		* submitted_byctrl if true(1) allow token submitted_by (and not just platform) to revoke and transfer.
		* Scope: token submitted_by
		*/
		TABLE currency_stats {
			asset		supply;
			asset		max_supply;
			name		issuer;
			uint64_t 	id;
			bool		submitted_byctrl;
			string		data;

			uint64_t primary_key()const {
				return supply.symbol.code().raw();
			}
		};
		typedef eosio::multi_index< "stat1"_n, currency_stats > stats;

		/*
		* Fungible token accounts table which stores information about balances.
		* Scope: token platform
		*/
		TABLE account {
			uint64_t	id;
			name		submitted_by;
			asset		balance;

			uint64_t primary_key()const {
				return id;
			}
		};

		typedef eosio::multi_index< "accounts1"_n, account > accounts;

		/*
		* Assets table which stores information about simple assets.
		* Scope: asset platform
		*/
		TABLE sasset {
			uint64_t                id;
			name                    platform; // platform
			name                    submitted_by; // submitted_by
			string                  idata; // immutable data
			string                  mdata; // mutable data
			string                  common_info; // asset detail ie. title
			string                  detail_info; //echo detail
			string                  ref_info; //platform info
			bool					revoke; // is revoke ?
			std::vector<sasset>     container;
			std::vector<account>    containerf;

			auto primary_key() const {
				return id;
			}

			uint64_t by_submitted() const {
				return submitted_by.value;
			}

		};
		typedef eosio::multi_index< "asset5"_n, sasset,
			eosio::indexed_by< "submittedby"_n, eosio::const_mem_fun<sasset, uint64_t, &sasset::by_submitted > >
			> sassets;

		/*
		* Text Digests table keep digest record for unique checking each asset before create 
		* Scope: self
		*/

		TABLE stdigest {
			uint64_t                id;
			uint64_t                asset_id;
			checksum256				digest;

			auto primary_key() const {
				return id;
			}
			checksum256 get_digest() const {
				return digest;
			}
			
			uint64_t get_asset() const {
				return asset_id;
			}
		};
		typedef eosio::multi_index< "stdg2"_n, stdigest,
			eosio::indexed_by< "digest"_n, eosio::const_mem_fun<stdigest, checksum256, &stdigest::get_digest> >,
			eosio::indexed_by< "asset"_n, eosio::const_mem_fun<stdigest, uint64_t, &stdigest::get_asset> >
			> stextdigests;

		/*
		* Image Digests table keep digest record for unique checking each asset before create 
		* Scope: self
		*/

		TABLE sidigest {
			uint64_t                id;
			uint64_t                asset_id;
			checksum256				digest;

			auto primary_key() const {
				return id;
			}
			checksum256 get_digest() const {
				return digest;
			}
			uint64_t get_asset() const {
				return asset_id;
			}
		};
		typedef eosio::multi_index< "sidg2"_n, sidigest,
			eosio::indexed_by< "digest"_n, eosio::const_mem_fun<sidigest, checksum256, &sidigest::get_digest> >,
			eosio::indexed_by< "asset"_n, eosio::const_mem_fun<sidigest, uint64_t, &sidigest::get_asset> >
			> simagedigests;

		/*
		* Offers table keeps records of open offers of assets (ie. assets waiting to be claimed by their
		* intendend recipients. Scope: self
		*/
		TABLE soffer {
			uint64_t		asset_id;
			name			platform;
			name			offeredto;
			uint64_t		cdate;

			auto primary_key() const {
				return asset_id;
			}
			uint64_t by_platform() const {
				return platform.value;
			}
			uint64_t by_offeredto() const {
				return offeredto.value;
			}
		};
		typedef eosio::multi_index< "offers2"_n, soffer,
			eosio::indexed_by< "platform"_n, eosio::const_mem_fun< soffer, uint64_t, &soffer::by_platform > >,
			eosio::indexed_by< "offeredto"_n, eosio::const_mem_fun< soffer, uint64_t, &soffer::by_offeredto > >
			> offers;

		/*
		* Offers table keeps records of open offers of FT (ie. waiting to be claimed by their
		* intendend recipients. Scope: self
		*/
		TABLE sofferf {
			uint64_t		id;
			name			submitted_by;
			name			platform;
			asset			quantity;
			name			offeredto;
			uint64_t		cdate;

			auto primary_key() const {
				return id;
			}
			uint64_t by_platform() const {
				return platform.value;
			}
			uint64_t by_offeredto() const {
				return offeredto.value;
			}
		};
		typedef eosio::multi_index< "offerfs1"_n, sofferf,
			eosio::indexed_by< "platform"_n, eosio::const_mem_fun< sofferf, uint64_t, &sofferf::by_platform > >,
			eosio::indexed_by< "offeredto"_n, eosio::const_mem_fun< sofferf, uint64_t, &sofferf::by_offeredto > >
			> offerfs;

		/*
		* Delegates table keeps records about borrowed assets.Scope: self
		*/
		TABLE sdelegate {
			uint64_t		asset_id;
			name			platform;
			name			delegatedto;
			uint64_t		cdate;
			uint64_t		period;
			string			memo;

			auto primary_key() const {
				return asset_id;
			}
			uint64_t by_platform() const {
				return platform.value;
			}
			uint64_t by_delegatedto() const {
				return delegatedto.value;
			}
		};
		typedef eosio::multi_index< "delegates1"_n, sdelegate,
			eosio::indexed_by< "platform"_n, eosio::const_mem_fun< sdelegate, uint64_t, &sdelegate::by_platform > >,
			eosio::indexed_by< "delegatedto"_n, eosio::const_mem_fun< sdelegate, uint64_t, &sdelegate::by_delegatedto > >
		> delegates;

		/*
		* global singelton table, used for asset_id building. Scope: self
		*/
		TABLE global {
			global() {}
			uint64_t lnftid = 100000000000000;
			uint64_t defid = 1000000;
			uint64_t textid = 100000000000000;
			uint64_t imageid = 100000000000000;

			EOSLIB_SERIALIZE( global, ( lnftid )( defid )( textid )( imageid ) )
		};

		typedef eosio::singleton< "global"_n, global > conf; /// singleton
		global _cstate; /// global state

		/*
		* Helps external contracts parse actions and tables correctly (Usefull for decentralized exchanges,
		* marketplaces and other contracts that use multiple tokens)
		* Marketplaces, exchanges and other reliant contracts will be able to view this info using the following code.
		*   Configs configs("simpl1assets"_n, "simpl1assets"_n.value);
		*	configs.get("simpl1assets"_n);
		*/
		TABLE tokenconfigs {
			name			standard;
			std::string		version;
		};
		typedef singleton< "tokenconfigs"_n, tokenconfigs > Configs;
};

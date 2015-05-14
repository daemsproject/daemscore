/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var MyWallet = new function() {
    var MyWallet = this;

    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view
    var password; //Password
    var dpassword; //double encryption Password
    var dpasswordhash; //double encryption Password
    var accountID;
    var final_balance = 0; //Final Satoshi wallet balance
    var total_sent = 0; //Total Satoshi sent
    var total_received = 0; //Total Satoshi received
    var n_tx = 0; //Number of transactions
    var n_tx_filtered = 0; //Number of transactions after filtering
    var latest_block; //Chain head block
    var address_book = {}; //Holds the address book addr = label
    var transactions = []; //List of all transactions (initially populated from /multiaddr updated through websockets)
    
    var tx_page = 0; //Multi-address page
    var tx_filter = 0; //Transaction filter (e.g. Sent Received etc)
    var maxAddr = 1000; //Maximum number of addresses
    var IDs = []; //{addr : address, priv : private key, tag : tag (mark as archived), label : label, balance : balance}
    
    var archTimer; //Delayed Backup wallet timer
    var recommend_include_fee = true; //Number of unconfirmed transactions in blockchain.info's memory pool
    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)
    var last_input_main_password; //The time the last password was entered    
    var isInitialized = false;    
    var language = 'en'; //Current language    
    var haveBoundReady = false;
    var isRestoringWallet = false;
    var sync_pubkeys = false;    
    var wallet_options = {    
        fee_policy : 0,  //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications : false, //HTML 5 Desktop notifications    
        tx_display : 0, //Compact or detailed transactions    
        transactions_per_page : 30, //Number of transactions per page    
    };
    this.setNTransactionsPerPage = function(val) {
        wallet_options.transactions_per_page = val;
    }

    this.getNTransactionsPerPage = function() {
        return wallet_options.transactions_per_page;
    }
    function bindInitial() {       

        $('.modal').on('show', function() {
            hidePopovers();

            $(this).center();
        }).on('hidden', function () {
                var visible = $('.modal:visible');

                var notices = $('#notices').remove();

                if (visible.length > 0)
                    visible.find('.modal-body').prepend(notices);
                else
                    $('#main-notices-container').append(notices);

            }).on('shown', function() {
                hidePopovers();

                var self = $(this);
                setTimeout(function() {
                    if (self.is(':visible')) {
                        self.find('.modal-body').prepend($('#notices').remove());
                    }
                }, 100);

                self.center();
            });
    }
    function hidePopovers() {
        try {
            $('.popover').remove();
        } catch (e) {}
    }
    function buildHomeIntroView(reset) {
        
//        $('#summary-n-tx').html(n_tx);
//
//        $('#summary-received').html(formatMoney(total_received, true));
//
//        $('#summary-sent').html(formatMoney(total_sent, true));
//
//        $('#summary-balance').html(formatMoney(final_balance, symbol));

//        $('.paper-wallet-btn').unbind().click(function() {
//            loadScript('wallet/paper-wallet', function() {
//                PaperWallet.showModal();
//            });
//        });



    }
    function changeView(id) {
        if (id === cVisible)
            return;

        if (cVisible != null) {
            if ($('#' + cVisible.attr('id') + '-btn').length > 0)
                $('#' + cVisible.attr('id') + '-btn').parent().attr('class', '');

            cVisible.hide();
        }

        cVisible = id;

        cVisible.show();

        if ($('#' + cVisible.attr('id') + '-btn').length > 0)
            $('#' + cVisible.attr('id') + '-btn').parent().attr('class', 'active');

        buildVisibleView(true);
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }

        haveBoundReady = true;
        $("#home-intro-btn").click(function() {
            changeView($("#home-intro"));
        });

        $("#my-transactions-btn").click(function() {
            changeView($("#my-transactions"));
        });

        $("#send-coins-btn").click(function() {
            changeView($("#send-coins"));
        });
    }
    function updateWalletFeedback(walletJson){
        
    }
    this.get_history=function(success,error){
        BrowserAPI.get_history(accountID,function(data){
            if (!data||data.error){
                if (error) error();
                return;                
            }
                
            parseMultiAddressJSON(data, false);

            if (transactions.length == 0 && tx_page > 0) {
                //We have set a higher page number than transactions we actually have to display
                //So rewind the page number to 0
                MyWallet.setPage(0);
            } else {
                //Rebuild the my-addresses list with the new updated balances (Only if visible)
                buildVisibleView();
            }

            if (success) success();

        }, function() {
            if (error) error();

        }, tx_page*MyWallet.getNTransactionsPerPage(), MyWallet.getNTransactionsPerPage());
    };
    function parseMultiAddressJSON(obj, cached) {
        if (!cached) {
            recommend_include_fee = obj.recommend_include_fee;

            if (obj.info) {
                if (obj.info.symbol_local)
                    setLocalSymbol(obj.info.symbol_local);

                if (obj.info.symbol_btc)
                    setBTCSymbol(obj.info.symbol_btc);

                if (obj.info.notice)
                    MyWallet.makeNotice('error', 'misc-error', obj.info.notice);
            }
        }

        sharedcoin_endpoint = obj.sharedcoin_endpoint;

        transactions.length = 0;

        if (obj.wallet == null) {
            total_received = 0;
            total_sent = 0;
            final_balance = 0;
            n_tx = 0;
            n_tx_filtered = 0;
            return;
        }

        total_received = obj.wallet.total_received;
        total_sent = obj.wallet.total_sent;
        final_balance = obj.wallet.final_balance;
        n_tx = obj.wallet.n_tx;
        n_tx_filtered = obj.wallet.n_tx_filtered;

        for (var i = 0; i < obj.addresses.length; ++i) {
            if (addresses[obj.addresses[i].address])
                addresses[obj.addresses[i].address].balance = obj.addresses[i].final_balance;
        }


        for (var i = 0; i < obj.txs.length; ++i) {
            var tx = TransactionFromJSON(obj.txs[i]);

            //Don't use the result given by the api because it doesn't include archived addresses
            tx.result = calcTxResult(tx, false);

            transactions.push(tx);
        }

        if (!cached) {
            if (obj.info.latest_block)
                setLatestBlock(obj.info.latest_block);
        }

        MyWallet.sendEvent('did_multiaddr');
    }
    function registerNotifications(){
        BrowserAPI.regNotifyBlocks(this.notifiedBlock);
        BrowserAPI.regNotifyTxs(this.notifiedTx,IDs);
        BrowserAPI.regNotifyAccount(this.notifiedAccount);
        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.notifiedBlock=function(obj){
         for (var i = 0; i < obj.x.txIndexes.length; ++i) {
                        for (var ii = 0; ii < transactions.length; ++ii) {
                            if (transactions[ii].txIndex == obj.x.txIndexes[i]) {
                                if (transactions[ii].blockHeight == null || transactions[ii].blockHeight == 0) {
                                    transactions[ii].blockHeight = obj.x.height;
                                    break;
                                }
                            }
                        }
                    }

                    setLatestBlock(BlockFromJSON(obj.x));

                    MyWallet.sendEvent('on_block');

                    //Need to update latest block
                    buildTransactionsView();
    }
    this.notifiedTx=function(obj){
        var tx = TransactionFromJSON(obj.x);

                    //Check if this is a duplicate
                    //Maybe should have a map_prev to check for possible double spends
                    for (var key in transactions) {
                        if (transactions[key].txIndex == tx.txIndex)
                            return;
                    }

                    var result = calcTxResult(tx, true);

                    if (MyWallet.getHTML5Notifications()) {
                        //Send HTML 5 Notification
                        MyWallet.showNotification({
                            title : result > 0 ? 'Payment Received' : 'Payment Sent',
                            body : 'Transaction Value ' + formatBTC(result),
                            iconUrl : resource + 'cube48.png'
                        });
                    }

                    tx.result = result;

                    final_balance += result;

                    n_tx++;

                    tx.setConfirmations(0);

                    playSound('beep');

                    if (tx_filter == 0 && tx_page == 0) {
                        transactions.unshift(tx);

                        var did_pop = false;
                        if (transactions.length > MyWallet.getNTransactionsPerPage()) {
                            transactions.pop();
                            did_pop = true;
                        }
                    }

                    MyWallet.sendEvent('on_tx');

                    var id = buildVisibleViewPre();
                    if ("my-transactions" == id) {
                        if (tx_filter == 0 && tx_page == 0) {
                            $('#no-transactions').hide();

                            if (wallet_options.tx_display == 0) {
                                var txcontainer = $('#transactions-compact').show();

                                bindTx(getCompactHTML(tx, addresses, address_book), tx).prependTo(txcontainer.find('tbody')).find('div').hide().slideDown('slow');

                                if (did_pop) {
                                    txcontainer.find('tbody tr:last-child').remove();
                                }

                            } else {
                                var txcontainer = $('#transactions-detailed').show();

                                txcontainer.prepend(tx.getHTML(addresses, address_book));

                                if (did_pop) {
                                    txcontainer.find('div:last-child').remove();
                                }

                                setupSymbolToggle();
                            }
                        }
                    } else {
                        buildVisibleView();
                    }
    }
    this.notifiedAccount=function(data){
        
    }
    this.notifiedPeers=function(data){
        
    }
    //Reset is true when called manually with changeview
    function buildVisibleViewPre() {
        //Hide any popovers as they can get stuck whent the element is re-drawn
        hidePopovers();

        //Update the account balance
        if (final_balance == null) {
            $('#balance').html('Loading...');
        } else {
            //$('#balance').html(formatSymbol(final_balance, symbol, true));
            //$('#balance2').html(formatSymbol(final_balance, (symbol === symbol_local) ? symbol_btc : symbol_local), true);
        }

        //Only build when visible
        return cVisible.attr('id');
    }

    //Reset is true when called manually with changeview
    function buildVisibleView(reset) {
        
        var id = buildVisibleViewPre();

        if ("send-coins" == id)
            buildSendTxView(reset);
        else if ("home-intro" == id)
            buildHomeIntroView(reset);
        else if ("receive-coins" == id)
            buildReceiveCoinsView(reset)
        else if ("my-transactions" == id)
            buildTransactionsView(reset)
    }
    function initAccount(){
        accountID=BrowserAPI.getAccountID();          
        $("#account-id").html(accountID);
        IDs=BrowserAPI.getIDs(accountID);  
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        MyWallet.get_history();
    }
    
   $(document).ready(function() {

        if (!$.isEmptyObject({}) || !$.isEmptyObject([])) {
            MyWallet.makeNotice('error', 'error', 'Object.prototype has been extended by a browser extension. Please disable this extensions and reload the page.');
            return;
        }

        //Listener to reload the page on App Cache update
        window.applicationCache.addEventListener('updateready', function () {
            if(window.applicationCache.status === window.applicationCache.UPDATEREADY) {
                window.applicationCache.swapCache();
                location.reload();
            }
        });

        //Disable autocomplete in firefox
        $("input,button,select").attr("autocomplete","off");


        var body = $(document.body);
//        //Deposit pages set this flag so it can be loaded in an iframe
//        if (MyWallet.skip_init)
//            return;
        
         cVisible = $("#home-intro");
        bindInitial();  
        initAccount();
        
        //Frame break
        if (top.location != self.location) {
            top.location = self.location.href
        }

       

        cVisible.show();

        $(document).ajaxStart(function() {
                setLogoutImageStatus('loading_start');

                $('.loading-indicator').fadeIn(200);
            }).ajaxStop(function() {
                setLogoutImageStatus('loading_stop');

                $('.loading-indicator').hide();
            });
    });
}
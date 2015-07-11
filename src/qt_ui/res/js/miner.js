/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Miner = new function () {
    var Miner = this;

    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view
    var password; //Password
    var dpassword; //double encryption Password
    var dpasswordhash; //double encryption Password
    var accountID;
    var balance = {balance_available: 0, balance_uncofirmed: 0, balance_locked: 0, balance_total: 0}; //Final Satoshi wallet balance    
    var hashrate = 0;
    var kernelrevenue = 0;
    var kernelinterval = 1000 * 24 * 3600;
    var txs = []; //List of all transactions (initially populated from /multiaddr updated through websockets)
    var intervalID;
    var tx_page = 0; //Multi-address page
    var tx_filter = 0; //Transaction filter (e.g. Sent Received etc)
    var maxAddr = 1000; //Maximum number of addresses
    var IDs = []; //{addr : address, priv : private key, tag : tag (mark as archived), label : label, balance : balance}

    var archTimer; //Delayed Backup wallet timer

    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)

    var isInitialized = false;
    var language = 'en'; //Current language    
    var haveBoundReady = false;

    var sync_pubkeys = false;
    var wallet_options = {
        fee_policy: 0, //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications: false, //HTML 5 Desktop notifications    
        tx_display: 0, //Compact or detailed transactions    
        transactions_per_page: 1000, //Number of transactions per page    
    };
    this.setNTransactionsPerPage = function (val) {
        wallet_options.transactions_per_page = val;
    }

    this.getNTransactionsPerPage = function () {
        return wallet_options.transactions_per_page;
    }
    function bindInitial() {

        $('.modal').on('show', function () {
            hidePopovers();

            $(this).center();
        }).on('hidden', function () {
            var visible = $('.modal:visible');

            var notices = $('#notices').remove();

            if (visible.length > 0)
                visible.find('.modal-body').prepend(notices);
            else
                $('#main-notices-container').append(notices);

        }).on('shown', function () {
            hidePopovers();

            var self = $(this);
            setTimeout(function () {
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
        } catch (e) {
        }
    }
    this.makeNotice = function (type, id, msg, timeout) {

        if (msg == null || msg.length == 0)
            return;

        console.log(msg);

        var el = $('<div class="alert alert-block alert-' + type + '"></div>');

        el.text('' + msg);

        if ($('#' + id).length > 0) {
            el.attr('id', id);
            return;
        }

        $("#notices").append(el).hide().fadeIn(200);

        (function () {
            var tel = el;

            setTimeout(function () {
                tel.fadeOut(250, function () {
                    $(this).remove();
                });
            }, timeout ? timeout : 5000);
        })();
    }
    
    function showRevenues() {

        var htmlcontent = '<table class="well table table-striped">';
        htmlcontent += '<thead><tr><th colspan=2>Recent Revenues</th><th></th></tr></thead><tbody> ';
        var count = 0;
        for (i = 0; i < txs.length; i++) {
            var tx = txs[i];
//            if(!tx.amount)
//                continue;
            var c = new Date(tx.blocktime * 1000);
            if (!tx.blocktime)
                c = new Date();
            if (tx.category=="minted") {
                htmlcontent += ('<tr OnClick="Miner.showTx(\'' + tx.txid + '\')"><td>');
                htmlcontent += '<img src="../icons/tx_mined.png">';
                htmlcontent += ('</td><td>');
                htmlcontent += '<div><div style="float:left">' + dateToString(c) + '</div><div style="text-align:right">';
                htmlcontent += (tx.amount + "CCC");
                htmlcontent += '</div></div>';
                htmlcontent += '<div>' + showID(tx.address) + '</div></td></tr>';
                count++;
                if (count >= 5)
                    break;
            }
        }
        htmlcontent += '</tbody> ';
        $("#latest-tx").html(htmlcontent);
    }
    this.getMiningInfo = function () {
        var info = BrowserAPI.getMiningInfo();
        if (info.kernelrate)
            hashrate = info.kernelrate;
        if (info.kernelrevenueperday)
            kernelrevenue = info.kernelrevenueperday;
        if (info.kernelblockinterval)
            kernelinterval = info.kernelblockinterval;
        if (info.kernels && info.kernels > 0) {
            $("#account-id").html(showID(accountID));
            $('select[name="kernels"]').html('');
            for (var i = 1; i <= info.kernels; i++)
                $('select[name="kernels"]').prepend('<option value="' + i + '" >' + i + '</option>');
            $('select[name="kernels"]').change();
        }
        if (info.networkhashrate)
            $("#net-hashrate").html(formatRate(info.networkhashrate));
        if (info.difficulty)
            $("#difficulty").html(info.difficulty.toFixed(3));
        if (info.connected)
            $("#connection").html("Yes");
        else
            $("#connection").html('<span style="color:red">Not Connected</span>');
        if (!info.generate)
            stopMining();
        else {
            $('select[name="kernels"]').val(info.threadsrunning);
            startMining();
        }
    }
    function formatRate(rate) {
        var str;
        if (rate >= 1000000000) {
            str = (rate / 1000000000).toFixed(3);
            str += " GHash/s";
        }
        else if (rate >= 1000000) {
            str = (rate / 1000000).toFixed(3);
            str += " MHash/s";
        }
        else if (rate >= 1000) {
            str = (rate / 1000).toFixed(3);
            str += " KHash/s";
        }
        else {
            str = rate.toFixed(3);
            str += " Hash/s";
        }

        return str;
    }
    function formatTime(time) {
        var str;
        if (time < 60) {
            str = time.toFixed(3);
            str += " seconds";
        }
        else if (time < 60 * 60) {
            str = (time / 60).toFixed(3);
            str += " minutes";
        }
        else if (time < 60 * 60 * 24) {
            str = (time / 3600).toFixed(3);
            str += " hours";
        }
        else {
            str = (time / (3600 * 24)).toFixed(3);
            str += " days";
        }
        return str;
    }

    this.get_history = function (success, error) {
        BrowserAPI.get_history(accountID, function (data) {
            if (!data || data.error) {
                if (error)
                    error();
                return;
            }
            console.log(data);
            txs = data.txs;
            for (var j in txs)
                txs[j] = parseTx(txs[j],IDs);
            balance = data.balance;
            if (txs.length == 0 && tx_page > 0) {
                //We have set a higher page number than transactions we actually have to display
                //So rewind the page number to 0
                Miner.setPage(0);
            } else {
                //Rebuild the my-addresses list with the new updated balances (Only if visible)
                showRevenues();
            }


            if (success)
                success();

        }, function () {
            if (error)
                error();

        }, tx_page * Miner.getNTransactionsPerPage(), Miner.getNTransactionsPerPage());
    };

    function registerNotifications() {
        //var aa=fuction(a){(a);};
//        var aa=function(a){
//            MyWallet.notifiedBlock(a);
//        };
        var ab = function (a) {
            Miner.notifiedTx(a);
        };
        var ad=function(a){
            Miner.notifiedID(a);
        };
        var af = function (a) {
            MyWallet.notifiedFallback(a);
        };
        //BrowserAPI.regNotifyBlocks(aa);        
        BrowserAPI.regNotifyTxs(ab, IDs);
        BrowserAPI.regNotifyID(ad);
        BrowserAPI.regNotifyFallback(af);
//        BrowserAPI.regNotifyAccount(this.notifiedAccount);
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.notifiedTx = function (a) {
        console.log("notified tx");
        for(var j in txs)
            if(txs[j].txid==a.tx.txid)
                return;
        var tx=parseTx(a.tx,IDs);
        txs.unshift(tx);
        //this.get_history();
        showRevenues();
    };
    this.notifiedFallback = function (obj) {
        console.log("notified fallback");
        i.get_history();
    }
    this.notifiedAccount = function (data) {

    }
    this.notifiedPeers = function (data) {

    }
    this.notifiedID=function(a){
        console.log(a);
        for(var j in IDs)
            if(IDs[j]==a.id)
                return;
        IDs.push(a.id);
        registerNotifications();       
    };
    //Reset is true when called manually with changeview
    function buildVisibleViewPre() {
        //Hide any popovers as they can get stuck whent the element is re-drawn
        hidePopovers();

        //Update the account balance
        if (balance == null) {
            //$('#balance').html('Loading...');
        } else {
            //$('#balance').html(formatSymbol(final_balance, symbol, true));
            //$('#balance2').html(formatSymbol(final_balance, (symbol === symbol_local) ? symbol_btc : symbol_local), true);
        }

        //Only build when visible
        return cVisible.attr('id');
    }
    var stopMining = function () {
        $('select[name="kernels"]').prop("disabled", false);
        $('.btn-primary').html("Start Mining");
        clearInterval(intervalID);
        $("#mining_status").html("");
    }
    var startMining = function () {
        clearInterval(intervalID);
        $('select[name="kernels"]').prop("disabled", true);
        $('.btn-primary').html("Stop Mining");
        $("#mining_status").html("Mining  CCCoins...");
        intervalID = setInterval(changeColor, 1000);
    }
    var color = "blue";
    var changeColor = function () {
        if (color == "blue")
            color = "grey";
        else
            color = "blue";
        document.getElementById("mining_status").style.color = color;
    }
    function bindReady() {
        if (haveBoundReady) {
            return;
        }

        haveBoundReady = true;




        $('.btn-primary').unbind().click(function () {
            if ($(this).html() == "Start Mining") {
                BrowserAPI.setGenerate(true, accountID, $('select[name="kernels"]').val(),true ,function () {
                    startMining();
                    Miner.makeNotice('success', 'start-success', 'Mining started');
                }, function (e) {
                    console.log(e);
                    stopMining();
                    Miner.makeNotice('error', 'send-tx-error', e);
                });
            }
            else
                BrowserAPI.setGenerate(false, accountID, $('select[name="kernels"]').val(),true, function () {
                    stopMining();
                });

        });
        $('select[name="kernels"]').unbind().change(function () {
            $('#hashrate').html(formatRate($(this).val() * hashrate));
            $('#estimated_revenue_amount').html(($(this).val() * kernelrevenue).toFixed(3) + " CCC/day");
            $('#estimated_revenue_interval').html(formatTime(kernelinterval / $(this).val()));
        }).trigger("change");
    }

    function initAccount() {
        accountID = BrowserAPI.getAccountID();
        $("#account-id").html(accountID);
        IDs=BrowserAPI.getIDs(accountID);  
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        Miner.get_history();
        setInterval(Miner.getMiningInfo(), 600000);
    }


    $(document).ready(function () {

        if (!$.isEmptyObject({}) || !$.isEmptyObject([])) {
            MyWallet.makeNotice('error', 'error', 'Object.prototype has been extended by a browser extension. Please disable this extensions and reload the page.');
            return;
        }

        //Listener to reload the page on App Cache update
        window.applicationCache.addEventListener('updateready', function () {
            if (window.applicationCache.status === window.applicationCache.UPDATEREADY) {
                window.applicationCache.swapCache();
                location.reload();
            }
        });

        //Disable autocomplete in firefox
        $("input,button,select").attr("autocomplete", "off");


        var body = $(document.body);
//        //Deposit pages set this flag so it can be loaded in an iframe
//        if (MyWallet.skip_init)
//            return;

        cVisible = $("#home-intro");
        bindInitial();
        initAccount();
        bindReady();
        //Frame break
        if (top.location != self.location) {
            top.location = self.location.href
        }



        cVisible.show();

        $(document).ajaxStart(function () {
            setLogoutImageStatus('loading_start');

            $('.loading-indicator').fadeIn(200);
        }).ajaxStop(function () {
            setLogoutImageStatus('loading_stop');

            $('.loading-indicator').hide();
        });
    });
}
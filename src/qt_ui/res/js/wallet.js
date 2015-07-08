/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var MyWallet = new function () {
    var i = this;

    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view

    var accountID;
    var balance = {balance_available: 0, balance_uncofirmed: 0, balance_locked: 0, balance_total: 0}; //Final Satoshi wallet balance
    var total_sent = 0; //Total Satoshi sent
    var total_received = 0; //Total Satoshi received
    var n_tx = 0; //Number of transactions
    var n_tx_filtered = 0; //Number of transactions after filtering
    var latest_block = {}; //Chain head block
    var address_book = {}; //Holds the address book addr = label
    var txs = []; //List of all transactions (initially populated from /multiaddr updated through websockets)

    var tx_page = 0; //Multi-address page
    var tx_filter = 0; //Transaction filter (e.g. Sent Received etc)
    var maxAddr = 1000; //Maximum number of addresses
    var IDs = []; //{addr : address, priv : private key, tag : tag (mark as archived), label : label, balance : balance}
    var LOCKTIME_THRESHOLD = 500000000;
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
    function buildHomeIntroView(reset) {
        var primary_address = $('#account-id');
        //if (primary_address.text() != preferred) {
        //  primary_address.text(preferred);

        loadScript('js/jquery.qrcode', function () {
            $('#my-primary-addres-qr-code').empty().qrcode({width: 125, height: 125, text: primary_address.text()})
        });
        //}
        $('#balance_available').html(balance.balance_available);
        $('#balance_unconfirmed').html(balance.balance_unconfirmed);
        $('#balance_locked').html(balance.balance_locked);
        $('#balance_total').html(balance.balance_total);
        var htmlcontent = '<table class="well table table-striped">';
        htmlcontent += '<thead><tr><th colspan=2>Recent Transactions</th><th></th></tr></thead><tbody> ';
        for (i = 0; i < Math.min(txs.length, 3); i++) {
            var tx = txs[i];
//            if(!tx.amount)
//                continue;
            var c = new Date(tx.blocktime * 1000);
            if (!tx.blocktime)
                c = new Date();
            htmlcontent += ('<tr OnClick="MyWallet.showTx(\'' + tx.txid + '\')"><td>');
            if (tx.generated)
                htmlcontent += '<img src="../icons/tx_mined.png">';
            else {
                switch (tx.category) {
                    case "send":
                        htmlcontent += '<img src="../icons/tx_output.png">';
                        break;
                    case "receive":
                        htmlcontent += '<img src="../icons/tx_input.png">';
                        break;
                    case "generate":
                        htmlcontent += '<img src="../icons/tx_mined.png">';
                        break;
                    default:
                        htmlcontent += '<img src="../icons/tx_inout.png">';

                }
            }
            htmlcontent += ('</td><td>');
            htmlcontent += '<div><div style="float:left">' + dateToString(c) + '</div><div style="text-align:right">';
            if (tx.category == "send")
                htmlcontent += '<font color="red">';
            htmlcontent += (tx.amount + "CCC");
            if (tx.category == "send")
                htmlcontent += '</font>';
            htmlcontent += '</div></div>';
            if (!tx.address)
                tx.address = "Publishing content";
            htmlcontent += ('<div>' + showID(tx.address) + '</div></td></tr>');
        }
        htmlcontent += '</tbody> ';
        $("#latest-tx").html(htmlcontent);


//        $('.paper-wallet-btn').unbind().click(function() {
//            loadScript('wallet/paper-wallet', function() {
//                PaperWallet.showModal();
//            });
//        });



    }
    function buildSendTxView(reset) {
        console.log("switch to send tx");
        $('#send-coins').show();
        $('#send-coins').find('.tab-pane.active').trigger('show', reset);

        if (reset) {
            //BlockchainAPI.get_ticker();

            $('.send').prop('disabled', false);
        }
    }
    function getActiveLabels() {
        var labels = [];
        for (var key in address_book) {
            labels.push(address_book[key]);
        }
        for (var key in IDs) {
            var addr = key;
            if (addr.tag != 2 && addr.label)
                labels.push(addr.label);
        }
        return labels;
    }
    function buildSendForm(el, reset) {


        el.find('.local-symbol').text(symbol_local.symbol);

        el.find('.btc-symbol').text(symbol_btc.symbol);

        if (reset) {
            el.find('input').val('');
            el.find('.send-value-usd').text(formatSymbol(0, symbol_local)).val('');
            el.find('.amount-needed').text(0);
        }

        var recipient_container = el.find(".recipient-container");

        if (reset) {
            var first_child = recipient_container.find(".recipient:first-child").clone();

            recipient_container.empty().append(first_child);
        }


        function bindRecipient(recipient) {

            recipient.find('input[name="send-to-address"]').typeahead({
                source: getActiveLabels()
            }).next().unbind().click(function () {
                var input = $(this).prev();
                BrowserAPI.scanQRCode(function (data) {
                    input.val(data);
                }, function (e) {
                    MyWallet.makeNotice('error', 'misc-error', e);
                });
            });

            recipient.find('.send-value').unbind().bind('keyup change', function (e) {
                if (e.keyCode == '9') {
                    return;
                }
            });
        }

        recipient_container.find(".recipient").each(function () {
            bindRecipient($(this));
        });

    }
    this.showTx = function (txid) {
        console.log(txid);
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
        $("#home-intro-btn").click(function () {
            changeView($("#home-intro"));
        });

        $("#my-transactions-btn").click(function () {
            changeView($("#my-transactions"));
        });

        $("#send-coins-btn").click(function () {
            changeView($("#send-coins"));
        });
        $('#send-quick').on('show', function (e, reset) {
            var self = $(this);
            buildSendForm(self, reset);
            self.find('.send').unbind().click(function () {
                $('.send').prop('disabled', true);
                BrowserAPI.requestPayment(accountID, self.find('input[name="send-to-address"]').val(), self.find('input[name="send-value"]').val(), self.find('textarea[name="send-message"]').val(), function () {
                    $('.send').prop('disabled', false);
                    self.find('input[name="send-to-address"]').val("");
                    self.find('input[name="send-value"]').val("");
                    self.find('textarea[name="send-message"]').val("");
                    MyWallet.makeNotice('success', 'send-tx-success', 'You payment is successfully sent');
                }, function (e) {
                    console.log(e);
                    $('.send').prop('disabled', false);
                    MyWallet.makeNotice('error', 'send-tx-error', e);
                });
            });
        });
    }

    function updateWalletFeedback(walletJson) {

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
            latest_block.blockHeight = data.currentblockheight;
            if (txs.length == 0 && tx_page > 0) {
                //We have set a higher page number than txs we actually have to display
                //So rewind the page number to 0
                MyWallet.setPage(0);
            } else {
                //Rebuild the my-addresses list with the new updated balances (Only if visible)
                buildVisibleView();
            }


            if (success)
                success();

        }, function () {
            if (error)
                error();

        }, tx_page * MyWallet.getNTransactionsPerPage(), MyWallet.getNTransactionsPerPage());
    };
    function setLatestBlock(block) {
        console.log(block);
        if (block != null) {
            latest_block = block;

            for (var j in txs) {
                var tx = txs[j];
                //console.log(latest_block.blockHeight);
                if (tx.blockheight != null && tx.blockheight > 0) {
                    //var confirmations = latest_block.blockHeight - tx.blockHeight + 1;
                    //if (confirmations <= 100) {
                    tx.confirmations = (latest_block.blockHeight - tx.blockheight + 1);
                    //console.log(tx.confirmations);
                    //} else {
                    //  tx.setConfirmations(null);
                    //}
                } else {
                    tx.confirmations = 0;
                }
            }

            //MyWallet.sendEvent('did_set_latest_block');
        }
    }

    function registerNotifications() {
        //var aa=fuction(a){(a);};
        var aa = function (a) {
            MyWallet.notifiedBlock(a);
        };
        var ab = function (a) {
            MyWallet.notifiedTx(a);
        };
        var ac = function (a) {
            //MyWallet.txs = [];
            //MyWallet.initAccount();
            window.location.href=window.location.href;
        };
        var ad = function (a) {
            MyWallet.notifiedID(a);
        };
        BrowserAPI.regNotifyBlocks(aa);
        BrowserAPI.regNotifyTxs(ab, IDs);
        BrowserAPI.regNotifyAccount(ac);
        BrowserAPI.regNotifyID(ad);
//        BrowserAPI.regNotifyAccount(this.notifiedAccount);
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.notifiedTx = function (a) {
        console.log(a);
         var b=BrowserAPI.getBalance(accountID);
        console.log(b);
        balance = b.balance;
        latest_block.blockHeight = b.currentblockheight;
        var tx=parseTx(a.tx,IDs);
        for(var j in txs)
            if(txs[j].txid==tx.txid){
                txs[j]=tx;
                buildVisibleView();
                return;
            }
        txs.unshift(tx);
       
        buildVisibleView();
        // this.get_history();
        //buildHomeIntroView();
        // buildTransactionsView();
    };
    this.notifiedBlock = function (obj) {
//         for (var i = 0; i < obj.x.txIndexes.length; ++i) {
//                        for (var ii = 0; ii < txs.length; ++ii) {
//                            if (txs[ii].txIndex == obj.x.txIndexes[i]) {
//                                if (txs[ii].blockHeight == null || txs[ii].blockHeight == 0) {
//                                    txs[ii].blockHeight = obj.x.height;
//                                    break;
//                                }
//                            }
//                        }
//                    }
        console.log("notified block");
        setLatestBlock(obj);

        //MyWallet.sendEvent('on_block');

        //Need to update latest block
        buildTransactionsView();
    }
    this.notifiedID = function (a) {
        console.log(a);
        for(var j in IDs)
            if(IDs[j]==a.id)
                return;
        IDs.push(a.id);
        registerNotifications();
    };

    this.notifiedAccount = function (data) {

    }
    this.notifiedPeers = function (data) {

    }
    function getLockIcon(tx) {
        var lockBlocks=0;
        var blocksLeft=0;
        var locktime=0;
        if(!tx.confirmations)
            return  'transaction0';
        for(var j in tx.vout)
            if(tx.vout[j].locktime>locktime)
                locktime=tx.vout[j].locktime;        
        if(locktime==0)    {     
            switch (tx.confirmations) {
                case 0:
                    return  'transaction0';                    
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    return  ('clock' + tx.confirmations);                    
                default:
                    return  'transaction2';
            }
        }
        if (locktime > LOCKTIME_THRESHOLD)//is time
        {
            var currentTime = (new Date()) / 1000;
            if (tx.blocktime)
                lockBlocks = (locktime - tx.blocktime) / 180;
            else
                lockBlocks = (locktime - currentTime) / 180;
            blocksLeft = (locktime - currentTime) / 180;

        } else//is blockheight
        {
            if (tx.blockheight > 0)
                lockBlocks = locktime - tx.blockheight;
            else
                lockBlocks = locktime - latest_block.blockHeight;
            blocksLeft = locktime - latest_block.blockHeight;
        }
        //console.log(lockBlocks);
        if (blocksLeft <= 0)
            return 'transaction2';
        if (blocksLeft > 480)
            return 'lock_closed';
        if (lockBlocks >= 480)
            lockBlocks = 480;
        
        //console.log(blocksLeft);
        var clock = "clock" + Math.ceil(((lockBlocks - blocksLeft) * 5 + 1) / lockBlocks);
        //console.log(clock);
        return clock;
    }    
    function getTxHTML(tx) {
        var tr = $('<tr class="pointer"></tr>');
        var html = '<td><img src="../icons/';        
       html += getLockIcon(tx);
        html += '.png"></td>';
        var c = new Date(tx.blocktime * 1000);
        if (!tx.blocktime)
            c = new Date();

        html += ('<td>' + dateToString(c) + '</td><td>');
//        if(tx.category=="send")
//            html+=('<font color="red">');
        //if (tx.category == "immature")
        //    tx.category = "generate";
        html += (tx.category);
//        if(tx.category=="send")
        html += ('</font>');
        html += ('</td><td>');
        if (!tx.address)
            tx.address = "Publishing content";
        html += (tx.address + '</td><td style="text-align:right">');
        if (tx.amount <0)
            html += ('<font color="red">');
        html += (tx.amount + " CCC");
        if (tx.category == "send")
            html += ('</font>');
        html += ('</td>');
        tr.html(html);
        return tr;
    }
    //Display The My Transactions view
    function buildTransactionsView() {
        var interval = null;
        var start = 0;
        console.log("buildtxview");
        if (interval != null) {
            clearInterval(interval);
            interval = null;
        }

        var txcontainer;
        //if (wallet_options.tx_display == 0) {
        $('#transactions-detailed').hide();
        txcontainer = $('#transactions-compact').show().find('tbody').empty();
        //} else {
        //    $('#transactions-compact').hide();
        //    txcontainer = $('#transactions-detailed').empty().show();
        //}

        if (txs.length == 0) {
            $('#transactions-detailed, #transactions-compact').hide();
            $('#no-transactions').show();
            return;
        } else {
            $('#no-transactions').hide();
        }

        var buildSome = function () {
            //for (var i = start; i < txs.length && i < (start+MyWallet.getNTransactionsPerPage()); ++i) {
            for (var i = start; i < txs.length; ++i) {
                var tx = txs[i];

                //if (wallet_options.tx_display == 0) {
                txcontainer.append(bindTx(getTxHTML(tx), tx));
                //} else {
                //    txcontainer.append(tx.getHTML(IDs, address_book));
                //}
            }

//            start += MyWallet.getNTransactionsPerPage();
//
//            if (start < txs.length) {
//                interval = setTimeout(buildSome, 15);
//            } else {
//                setupSymbolToggle();
//
//                hidePopovers();
//
//                var pagination = $('.pagination ul').empty();
//
//                if (tx_page == 0 && txs.length < MyWallet.getNTransactionsPerPage()) {
//                    pagination.hide();
//                    return;
//                } else {
//                    pagination.show();
//                }
//
//                var pages = Math.ceil(n_tx_filtered / MyWallet.getNTransactionsPerPage());
//
//                var disabled = ' disabled';
//                if (tx_page > 0)
//                    disabled = '';
//
//                var maxPagesToDisplay = 10;
//
//                var start_page = Math.max(0, Math.min(tx_page-(maxPagesToDisplay/2), pages-maxPagesToDisplay));
//
//                pagination.append($('<li class="prev'+disabled+'"><a>&larr; Previous</a></li>').click(function() {
//                    MyWallet.setPage(tx_page-1);
//                }));
//
//                if (start_page > 0) {
//                    pagination.append($('<li><a>≤</a></li>').click(function() {
//                        MyWallet.setPage(0);
//                    }));
//                }
//
//                for (var i = start_page; i < pages && i < start_page+maxPagesToDisplay; ++i) {
//                    (function(i){
//                        var active = '';
//                        if (tx_page == i)
//                            active = ' class="active"';
//
//                        pagination.append($('<li'+active+'><a class="hidden-phone">'+(i+1)+'</a></li>').click(function() {
//                            MyWallet.setPage(i);
//                        }));
//                    })(i);
//                }
//
//                if (start_page+maxPagesToDisplay < pages) {
//                    pagination.append($('<li><a>≥</a></li>').click(function() {
//                        MyWallet.setPage(pages-1);
//                    }));
//                }
//
//                var disabled = ' disabled';
//                if (tx_page < pages-1)
//                    disabled = '';
//
//                pagination.append($('<li class="next'+disabled+'"><a>Next &rarr;</a></li>').click(function() {
//                    MyWallet.setPage(tx_page+1)
//                }));
//            }
        };

        buildSome();
    }
    function bindTx(tx_tr, tx) {
        tx_tr.click(function () {
            var a = 1;
            // openTransactionSummaryModal(tx.txIndex, tx.result);
        });

//        tx_tr.find('.show-note').unbind('mouseover').mouseover(function() {
//            var note = tx.note ? tx.note : tx_notes[tx.hash];
//            showNotePopover(this, note, tx.hash);
//        });
//
//        tx_tr.find('.add-note').unbind('mouseover').mouseover(function() {
//            addNotePopover(this, tx.hash);
//        });

        return tx_tr;
    }
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
    this.initAccount=function() {        
        accountID = BrowserAPI.getAccountID();
        $("#account-id").html(accountID);
        console.log(accountID);
        IDs = BrowserAPI.getIDs(accountID);
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        MyWallet.get_history();
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
        i.initAccount();
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
/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Miner = new function () {
    var Miner = this;

    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view
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
    var info = {};
    var haveBoundReady = false;
    var wallet_options = {
        fee_policy: 0, //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications: false, //HTML 5 Desktop notifications    
        tx_display: 0, //Compact or detailed transactions    
        transactions_per_page: 5, //Number of transactions per page    
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
                    $("#notices").hide();
                });
            }, timeout ? timeout : 5000);
        })();
    }

    function showRevenues() {

        var htmlcontent = '<table class="well table table-striped">';
        htmlcontent += '<thead><tr><th colspan=2>' + TR('Recent Revenues') + '</th><th></th></tr></thead><tbody> ';
        var count = 0;
        for (i = 0; i < txs.length; i++) {
            var tx = txs[i];
//            if(!tx.amount)
//                continue;
            var c = new Date(tx.blocktime * 1000);
            if (!tx.blocktime)
                c = new Date();
            if (tx.category == "minted") {
                htmlcontent += ('<tr OnClick="Miner.showTx(\'' + tx.txid + '\')"><td>');
                htmlcontent += '<img src="../icons/tx_mined.png">';
                htmlcontent += ('</td><td>');
                htmlcontent += '<div><div style="float:left">' + CUtil.dateToString(c) + '</div><div style="text-align:right">';
                htmlcontent += ("φ" + tx.amount);
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
        info = FAI_API.getMiningInfo();
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
            $("#connection").html(TR('Connected'));
        else
            $("#connection").html('<span style="color:red">' + TR('Not Connected') + '</span>');
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

    this.get_history = function (success, error) {
        FAI_API.listtransactions(accountID, function (data) {
            if (!data || data.error) {
                if (error)
                    error();
                return;
            }
            txs = data.txs;
            for (var j in txs)
                txs[j] = parseTx(txs[j], IDs);
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
        var ab = function (a) {
            Miner.notifiedTx(a);
        };
        var ad = function (a) {
            Miner.notifiedID(a);
        };
        var af = function (a) {
            Miner.notifiedFallback(a);
        };
        var an = function (a) {
            Miner.notifiedAccount(a);
        };
        var aa = function (a) {
            CPage.notifyBlock(a);
        };
        FAI_API.regNotifyTxs(ab, IDs);
        FAI_API.regNotifyID(ad);
        FAI_API.regNotifyFallback(af);
        FAI_API.regNotifyAccount(an);
        FAI_API.regNotifyBlocks(aa);
    }
    this.notifiedTx = function (a) {
        for (var j in txs)
            if (txs[j].txid == a.tx.txid)
                return;
        var tx = parseTx(a.tx, IDs);
        txs.unshift(tx);
        showRevenues();
        CPage.updateBalance(FAI_API.getBalance(accountID).balance);
    };
    this.notifiedFallback = function (obj) {
        i.get_history();
    }
    this.notifiedAccount = function (data) {
        accountID = FAI_API.getAccountID();
        $("#account-id").html(showID(accountID));
        IDs = FAI_API.getIDs(accountID);
        registerNotifications();
        Miner.get_history(function(){CPage.updateBalance(balance);});
        if (info.generate)
            FAI_API.setGenerate(true, accountID, $('select[name="kernels"]').val());
    }
    this.notifiedPeers = function (data) {
    }
    this.notifiedID = function (a) {
        for (var j in IDs)
            if (IDs[j] == a.id)
                return;
        IDs.push(a.id);
        registerNotifications();
    };
    var stopMining = function () {
        $('select[name="kernels"]').prop("disabled", false);
        $('#mine').html(TR('Start Mining'));
        clearInterval(intervalID);
        $("#mining_status").html("");
    }
    var startMining = function () {
        clearInterval(intervalID);
        $('select[name="kernels"]').prop("disabled", true);
        $('#mine').html(TR('Stop Mining'));
        $("#mining_status").html(TR('Mining  FAIcoins...'));
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
        if (haveBoundReady) 
            return;

        haveBoundReady = true;
        $('#mine').unbind().click(function () {
            if ($(this).html() == TR('Start Mining')) {
                FAI_API.setGenerate(true, accountID, $('select[name="kernels"]').val(), false, function () {
                    startMining();
                    Miner.makeNotice('success', 'start-success', TR('Mining started'));
                }, function (e) {
                    console.log(e);
                    stopMining();
                    Miner.makeNotice('error', 'send-tx-error', TR(e));
                });
            }
            else
                FAI_API.setGenerate(false, accountID, $('select[name="kernels"]').val(), false, function () {
                    stopMining();
                });
        });
        $('select[name="kernels"]').unbind().change(function () {
            $('#hashrate').html(formatRate($(this).val() * hashrate));
            $('#estimated_revenue_amount').html("φ" + ($(this).val() * kernelrevenue).toFixed(3) + "/" + TR('day'));
            $('#estimated_revenue_interval').html(CUtil.formatTimeLength(kernelinterval / $(this).val()));
        }).trigger("change");
    }

    function initAccount() {
        accountID = FAI_API.getAccountID();
        $("#account-id").html(showID(accountID));
        IDs = FAI_API.getIDs(accountID);
        registerNotifications();
        Miner.get_history(function(){CPage.updateBalance(balance);});
        setInterval(Miner.getMiningInfo(), 600000);
    }


    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {
            CUtil.initGParam();
            CPage.prepareHeader(true);
            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            cVisible = $("#home-intro");
            bindInitial();
            initAccount();
            bindReady();
            cVisible.show();
            CPage.prepareNotice("miner");
            CPage.updateCblc();
        });
    });
}
/* 
 * courtesy of www.blockchain.info
 */
var MyWallet = new function () {
    var i = this;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view
    var accountID;
    var balance = {balance_available: 0, balance_uncofirmed: 0, balance_locked: 0, balance_total: 0}; //Final Satoshi wallet balance
    var n_tx = 0; //Number of transactions
    var n_tx_filtered = 0; //Number of transactions after filtering
    var latest_block = {}; //Chain head block
    var address_book = []; //Holds the address book addr = label
    var txs = []; //List of all transactions (initially populated from /multiaddr updated through websockets)

    var tx_page = 0; //Multi-address page
    var tx_filter = 0; //Transaction filter (e.g. Sent Received etc)
    var maxAddr = 1000; //Maximum number of addresses
    var IDs = []; //{addr : address, priv : private key, tag : tag (mark as archived), label : label, balance : balance}
    var LOCKTIME_THRESHOLD = 500000000;
    var recommend_include_fee = true; //Number of unconfirmed transactions in blockchain.info's memory pool
    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)

    var isInitialized = false;
    var language = 'en'; //Current language    
    var haveBoundReady = false;
    var sync_pubkeys = false;
    var wallet_options = {
        fee_policy: 0, //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications: false, //HTML 5 Desktop notifications    
        tx_display: 0, //Compact or detailed transactions    
        transactions_per_page: 30, //Number of transactions per page    
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
    function buildHomeIntroView(reset) {
        var primary_address = $('#account-id');
        loadScript('js/jquery.qrcode', function () {
            $('#my-primary-addres-qr-code').empty().qrcode({width: 125, height: 125, text: primary_address.text()})
        });
        $('#balance_available').html(balance.balance_available);
        $('#balance_unconfirmed').html(balance.balance_unconfirmed);
        $('#balance_locked').html(balance.balance_locked);
        $('#balance_total').html(balance.balance_total);
        var htmlcontent = '<table class="well table table-striped">';
        htmlcontent += '<thead><tr><th colspan=2>' + TR('Recent Transactions') + '</th><th></th></tr></thead><tbody> ';
        for (i = 0; i < Math.min(txs.length, 3); i++) {
            var tx = txs[i];
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
            htmlcontent += '<div><div style="float:left">' + CUtil.dateToString(c) + '</div><div style="text-align:right">';
            if (tx.category == "send")
                htmlcontent += '<font color="red">';
            htmlcontent += ("φ" + tx.amount);
            if (tx.category == "send")
                htmlcontent += '</font>';
            htmlcontent += '</div></div>';
            if (!tx.address)
                tx.address = TR('Publishing content');
            htmlcontent += ('<div>' + showID(tx.address) + '</div></td></tr>');
        }
        htmlcontent += '</tbody> ';
        $("#latest-tx").html(htmlcontent);
    }
    function buildSendTxView(reset) {
        console.log("switch to send tx");
        $('#send-coins').show();
        $('#send-coins').find('.tab-pane.active').trigger('show', reset);
        if (reset) {
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
        if (reset) {
            el.find('input').val('');
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
                FAI_API.scanQRCode(function (data) {
                    input.val(data);
                }, function (e) {
                    CPage.showNotice(TR("Scan QR code error"));
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
                var feerate = Number(FAI_API.getFeeRate(0.15));
                var locktime = 0;
                FAI_API.requestPayment(accountID, $.trim(self.find('input[name="send-to-address"]').val()), self.find('input[name="send-value"]').val(), self.find('textarea[name="send-message"]').val(), feerate, locktime, function () {
                    $('.send').prop('disabled', false);
                    self.find('input[name="send-to-address"]').val("");
                    self.find('input[name="send-value"]').val("");
                    self.find('textarea[name="send-message"]').val("");
                    CPage.showNotice(TR('You payment is successfully sent'));
                }, function (e) {
                    console.log(e);
                    $('.send').prop('disabled', false);
                    CPage.showNotice(TR("Failed to send tx: ") + TR(e));
                });
            });
        });
        $("#tx-detail-modal").find(".cancel").unbind().click(function () {
            $("#tx-detail-modal").modal("hide");
        });
        $("#addressbook-btn").click(function () {
            changeView($("#addressbook"));
            MyWallet.loadAddressBook();
            MyWallet.showAddressBook();
        });
    }
    this.loadAddressBook = function () {
        var adfile = FAI_API.readFile("wallet", "addressbook", "adb.json");
        address_book = $.parseJSON(adfile);
        if (!address_book)
            address_book = [];
    }
    this.saveAddressBook = function () {
        var adfile = JSON.stringify(address_book);
        FAI_API.writeFile("wallet", "addressbook", "adb.json", adfile);
    }
    this.showAddressBook = function () {
        var html = "<tr><th>" + TR('Address') + "</th><th>" + TR('Alias') + "</th><th>" + TR('Action') + "</th></tr>";
        for (var i in address_book) {
            var ad = address_book[i];
            if (!ad.alias)
                ad.alias = "";
            html += '<tr><td><input  name="' + ad.id + '"   value="' + ad.id + '" readOnly="readOnly"/></td>';
            html += '<td><input  name="' + ad.id + '_alias"   value="' + ad.alias + '"/></td>';
            html += '<td>';
            html += '<button onclick="MyWallet.changeAdbItem(' + "'" + ad.id + "'" + ')">' + TR('Change') + '</button>';
            html += '<button onclick="MyWallet.deleteAdbItem(' + "'" + ad.id + "'" + ',1)">' + TR('Delete') + '</button>';
            html += '</td></tr>';
        }
        html += '<tr><td><input  name="newid"/></td>';
        html += '<td><input  name="newtag"/></td>';
        html += '<td>';
        html += '<button onclick="MyWallet.addAdbItem()">' + TR('Add') + '</button>';
        html += '</td></tr>';
        $("#table_addressbook").html(html);
    }
    this.addAdbItem = function () {
        var ad = {};
        ad.id = $("#table_addressbook").find("input[name='newid']").val();
        ad.alias = $("#table_addressbook").find("input[name='alias']").val();
        if (!ad.alias)
            ad.alias = "";
        if (ad.id.length == 0) {
            CPage.showNotice(TR('empty id.'));
            return false;
        }
        for (var i in address_book) {
            if (address_book[i].id == ad.id) {
                CPage.showNotice(TR('duplicate id.'));
                return false;
            }
            if (ad.alias && address_book[i].alias && address_book[i].alias == ad.alias) {
                CPage.showNotice(TR('duplicate alias.'));
                return false;
            }
        }
        address_book.push(ad);
        MyWallet.saveAddressBook();
        MyWallet.showAddressBook();
        CPage.showNotice(TR('Address added'));
    }
    this.changeAdbItem = function (id) {
        var ad = {};
        ad.id = $("#table_addressbook").find("input[name='" + id + "']").val();
        ad.alias = $("#table_addressbook").find("input[name='" + id + "_alias']").val();
        if (!ad.alias)
            ad.alias = "";
        for (var i in address_book) {
            if (address_book[i].id == ad.id) {
                continue;
            }
            if (ad.alias && address_book[i].alias && address_book[i].alias == ad.alias) {
                CPage.showNotice(TR('duplicate alias.'));
                return false;
            }
        }
        for (var i in address_book) {
            if (address_book[i].id == ad.id) {
                address_book[i].alias = ad.alias;
            }
        }
        MyWallet.saveAddressBook();
        MyWallet.showAddressBook();
        CPage.showNotice(TR('Address changed'));
    }
    this.deleteAdbItem = function (id) {
        var ad = {};
        ad.id = $("#table_addressbook").find("input[name='" + id + "']").val();

        for (var i in address_book) {
            if (address_book[i].id == ad.id) {
                address_book.splice(i, 1);
            }
        }
        MyWallet.saveAddressBook();
        MyWallet.showAddressBook();
        CPage.showNotice(TR('Address changed'));
    }
    this.get_history = function (success, error) {
        FAI_API.listtransactions(accountID, function (data) {
            if (!data || data.error) {
                if (error)
                    error();
                return;
            }
            console.log(data);
            txs = data.txs;
            for (var j in txs)
                txs[j] = parseTx(txs[j], IDs);
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

        });// ,MyWallet.getNTransactionsPerPage(), tx_page *MyWallet.getNTransactionsPerPage());
    };
    function setLatestBlock(block) {
        console.log(block);
        if (block != null) {
            latest_block = block;
            for (var j in txs) {
                var tx = txs[j];
                if (tx.blockheight != null && tx.blockheight > 0) {
                    tx.confirmations = (latest_block.blockHeight - tx.blockheight + 1);
                } else {
                    tx.confirmations = 0;
                }
            }
        }
    }
    function registerNotifications() {
        var aa = function (a) {
            MyWallet.notifiedBlock(a);
        };
        var ab = function (a) {
            MyWallet.notifiedTx(a);
        };
        var ac = function (a) {
            window.location.href = window.location.href;
        };
        var ad = function (a) {
            MyWallet.notifiedID(a);
        };
        var af = function (a) {
            MyWallet.notifiedFallback(a);
        };
        FAI_API.regNotifyBlocks(aa);
        FAI_API.regNotifyTxs(ab, IDs);
        FAI_API.regNotifyAccount(ac);
        FAI_API.regNotifyID(ad);
        FAI_API.regNotifyFallback(af);
    }
    this.notifiedTx = function (a) {
        var b = FAI_API.getBalance(accountID);
        balance = b.balance;
        CPage.updateBalance(balance);
        latest_block.blockHeight = b.currentblockheight;
        var tx = parseTx(a.tx, IDs);
        for (var j in txs)
            if (txs[j].txid == tx.txid) {
                txs[j] = tx;
                buildVisibleView();
                return;
            }
        txs.unshift(tx);
        buildVisibleView();
    };
    this.notifiedBlock = function (obj) {
        setLatestBlock(obj);
        buildVisibleView();
    }
    this.notifiedFallback = function (obj) {
        i.get_history();
    }
    this.notifiedID = function (a) {
        console.log(a);
        for (var j in IDs)
            if (IDs[j] == a.id)
                return;
        IDs.push(a.id);
        registerNotifications();
    };
    function getLockIcon(tx) {
        var lockBlocks = 0;
        var blocksLeft = 0;
        var locktime = 0;
        if (!tx.confirmations)
            return  'transaction0';
        for (var j in tx.vout)
            if (tx.vout[j].locktime > locktime)
                locktime = tx.vout[j].locktime;
        if (locktime == 0) {
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
        if (blocksLeft <= 0)
            return 'transaction2';
        if (blocksLeft > 480)
            return 'lock_closed';
        if (lockBlocks >= 480)
            lockBlocks = 480;

        var clock = "clock" + Math.ceil(((lockBlocks - blocksLeft) * 5 + 1) / lockBlocks);
        return clock;
    }
    function getTxHTML(tx) {
        var tr = $('<tr class="pointer"></tr>');
        var html = '<td><img class="txicon" src="../icons/';
        html += getLockIcon(tx);
        html += '.png"></td>';
        var c = tx.blocktime ? new Date(tx.blocktime * 1000) : new Date();
        html += ('<td>' + CUtil.dateToString(c) + '</td><td>');
        html += TR(tx.category);
        html += ('</font>');
        html += ('</td><td class="id-text">');
        if (!tx.address)
            tx.address = TR('Publishing content');
        if (tx.address.length > 60)
            html += (tx.address.substr(0, 57) + '...' + '</td><td style="text-align:right">');
        else
            html += (tx.address + '</td><td style="text-align:right">');
        if (tx.amount < 0)
            html += ('<font color="red">');
        html += ("φ" + tx.amount);
        if (tx.amount < 0)
            html += ('</font>');
        html += ('</td>');
        tr.html(html);
        return tr;
    }
    //Display The My Transactions view
    function buildTransactionsView() {
        var interval = null;
        var start = tx_page * MyWallet.getNTransactionsPerPage();
        console.log("buildtxview");
        if (interval != null) {
            clearInterval(interval);
            interval = null;
        }

        var txcontainer;
        $('#transactions-detailed').hide();
        txcontainer = $('#transactions-compact').show().find('tbody').empty();
        if (txs.length == 0) {
            $('#transactions-detailed, #transactions-compact').hide();
            $('#no-transactions').show();
            return;
        } else {
            $('#no-transactions').hide();
        }

        var buildSome = function () {
            for (var i = start; i < txs.length && i < (start + MyWallet.getNTransactionsPerPage()); ++i) {
                var tx = txs[i];
                txcontainer.append(bindTx(getTxHTML(tx), tx));
            }
            hidePopovers();
            var pagination = $('.pagination ul').empty();
            console.log(MyWallet.getNTransactionsPerPage());
            if (tx_page == 0 && txs.length < MyWallet.getNTransactionsPerPage()) {
                pagination.hide();
                return;
            } else {
                pagination.show();
            }

            var pages = Math.ceil(txs.length / MyWallet.getNTransactionsPerPage());
            console.log(pages);
            var disabled = ' disabled';
            if (tx_page > 0)
                disabled = '';

            var maxPagesToDisplay = 10;

            var start_page = Math.max(0, Math.min(tx_page - (maxPagesToDisplay / 2), pages - maxPagesToDisplay));

            pagination.append($('<li class="prev' + disabled + '"><a>&larr; ' + TR('Previous') + '</a></li>').click(function () {
                MyWallet.setPage(tx_page - 1);
            }));

            if (start_page > 0) {
                pagination.append($('<li><a>≤</a></li>').click(function () {
                    MyWallet.setPage(0);
                }));
            }

            for (var ii = start_page; ii < pages && ii < start_page + maxPagesToDisplay; ++ii) {
                (function (ii) {
                    var active = '';
                    if (tx_page == ii)
                        active = ' class="active"';

                    pagination.append($('<li' + active + '><a class="hidden-phone">' + (ii + 1) + '</a></li>').click(function () {
                        MyWallet.setPage(ii);
                    }));
                })(ii);
            }

            if (start_page + maxPagesToDisplay < pages) {
                pagination.append($('<li><a>≥</a></li>').click(function () {
                    MyWallet.setPage(pages - 1);
                }));
            }

            var disabled = ' disabled';
            if (tx_page < pages - 1)
                disabled = '';

            pagination.append($('<li class="next' + disabled + '"><a>' + TR('Next') + ' &rarr;</a></li>').click(function () {
                MyWallet.setPage(tx_page + 1)
            }));
        };

        buildSome();
    }
    this.setPage = function (npage) {
        tx_page = npage;
        scroll(0, 0);
        buildTransactionsView();
    };
    function bindTx(tx_tr, tx) {
        tx_tr.click(function () {
            openTransactionDetailModal(tx);
        });
        return tx_tr;
    }
    function getValueSpan(value) {
        console.log(value);
        return $("<span />").addClass("value").html("φ" + value);
    }
    function openTransactionDetailModal(tx)
    {
        if (!tx.blockheight || tx.blockheight <= 0) {
            $("#btn-override").show();
            $("#btn-override").unbind().click(function () {
                var feerate = Number(FAI_API.getFeeRate(0.15));
                FAI_API.requestOverride(accountID, tx.txid, feerate, "*", function (a) {
                    CPage.showNotice(TR(a));
                }, function (e) {
                    CPage.showNotice(TR("Override failed") + ": " + TR(e));

                });
            });
        }
        else
            $("#btn-override").hide();
        var m = $("#tx-detail-tpl").clone(true, true).removeAttr("id").removeClass("hide");
        m.find(".txid").append(tx.txid);
        m.find(".height").append(tx.blockheight);
        var t = tx.time ? new Date(tx.time * 1000) : new Date();
        m.find(".time").append(CUtil.dateToString(t));
        if (tx.category == "minted") {
            var vin = $("<div />").html(TR("Coinbase: ")).append(getValueSpan(tx.vin[0].value));
            m.find(".vins").append(vin);
        } else {
            for (var i in tx.vin) {
                var id = tx.vin[i].scriptPubKey.address;
                var idspan = $("<span />").html(id).attr("title", id);
                var vin = $("<div />").append(i).append(":").append(idspan).append(": ").append(getValueSpan(tx.vin[i].value));
                m.find(".vins").append(vin);
            }
        }
        for (var i in tx.vout) {
            var id2show = TR("No address");
            var id = "";
            if (tx.vout[i].scriptPubKey.address) {
                id2show = tx.vout[i].scriptPubKey.address;
                //id2show = CUtil.getLongPId(id);
            }
            var idspan = $("<span />").html(id2show).attr("title", id);

            var contentspan = $("<span />").html(tx.vout[i].content ? TR(" content len: ") + tx.vout[i].content.length / 2 : "");
            var timeleft = 0;
            var locktimespan = $("<span />");
            if (tx.vout[i].locktime) {
                timeleft = FAI_API.getMatureTime(tx.vout[i].locktime).time;
                console.log(FAI_API.getMatureTime(tx.vout[i].locktime));
            }
            console.log(timeleft);
            var locktimespan = $("<span />").html(timeleft > 0 ? TR(" unlock in: ") + CUtil.formatTimeLength(timeleft) : "");
            console.log(locktimespan);
            var vout = $("<div />").append(i).append(":").append(idspan).append(": ").append(getValueSpan(tx.vout[i].value)).append(contentspan).append(locktimespan);
            m.find(".vouts").append(vout);
        }
        $("#tx-detail-modal").find(".tx-details").html("").append(m);
        $("#tx-detail-modal").modal({show: true}).center();
    }
    function buildVisibleViewPre() {
        return cVisible.attr('id');
    }

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
    this.initAccount = function () {
        accountID = FAI_API.getAccountID();
        $("#account-id").html(accountID);
        console.log(accountID);
        IDs = FAI_API.getIDs(accountID);
        registerNotifications();
        MyWallet.get_history();
    }
    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {

            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            cVisible = $("#home-intro");
            bindInitial();
            i.initAccount();
            CUtil.initGParam(balance);
            bindReady();
            cVisible.show();
            CPage.prepareNotice("wallet");
            CPage.updateBalance();
            CPage.updateCblc();
            CPage.registerNotifications();
        });
    });
}
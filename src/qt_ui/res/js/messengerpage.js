var cVisible; //currently visible view    
//var balance = {};
var currentContact;
var contacts = {};
function registerNotifications() {
    var aa = function (a) {
        Messenger.notifiedBlock(a);
    };
    var ab = function (a) {
        Messenger.notifiedTx(a);
    };
    var ac = function (a) {
        window.location.href = window.location.href;
    }
    FAI_API.regNotifyBlocks(aa);
    FAI_API.regNotifyTxs(ab, [accountID]);
    FAI_API.regNotifyAccount(ac);
}

function changeCategory(id) {
    if (cVisible != null) {
        if ($('#' + cVisible.attr('id')).length > 0)
            $('#' + cVisible.attr('id')).parent().attr('class', '');
    }
    cVisible = id;
    if ($('#' + cVisible.attr('id')).length > 0)
        $('#' + cVisible.attr('id')).parent().attr('class', 'active');
    switch (id.attr('id')) {
        case "list-all":
            for (var j in contacts) {
                $("#" + j).hide();
                if (contacts[j].category != "blacklisted")
                    $("#" + j).show();
            }
            break;
        case "list-friends":
            for (var j in contacts) {
                $("#" + j).hide();
                if (contacts[j].category == "friend")
                    $("#" + j).show();
            }
            break;
        case "list-black":
            for (var j in contacts) {
                $("#" + j).hide();
                if (contacts[j].category == "blacklisted")
                    $("#" + j).show();
            }
            break;
    }
}

function initAccount() {
    accountID = FAI_API.getAccountID();
    $("#account-id").html(accountID);
    registerNotifications();
    Messenger.getMessagesAll();
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
$(document).ready(function () {
    $("#tpls").load("templates.html", function () {
        CUtil.initGParam();
        var page = Messenger.getPageName();
        CPage.prepareNotice("msgr");
        doTranslate();
        $("input,button,select").attr("autocomplete", "off");
        CPage.prepareHeader(true);
        CPage.updateBalance();
        CPage.updateCblc();
        $("#list-all").click(function () {
            changeCategory($("#list-all"));
        });
        $("#list-friends").click(function () {
            changeCategory($("#list-friends"));
        });
        $("#list-black").click(function () {
            changeCategory($("#list-black"));
        });

        $('#btn-send').unbind().click(function () {
            var msg = $("#theText").val();
            if (msg.length > 10000) {
                CPage.showNotice(TR('Message oversize.'));
                return;
            }
            if (currentContact && msg) {
                var feerate = FAI_API.getFeeRate(0.15);
                FAI_API.sendMessage(accountID, currentContact, msg, feerate, function () {
                    Messenger.showMessage(msg, currentContact, 0, "onchain", "out");
                    Messenger.setLastUpdateTime(currentContact);
                    $("#theText").val("");
                }, function (e) {
                    CPage.showNotice(TR('send-message-error: ') + TR(e));
                });
            }
        });
        $('#btn-add').unbind().click(function () {
            Messenger.showModal("add");
        });
        $("#edit-contact").find(".btn-secondary").unbind().click(function () {
            $("#edit-contact").modal("hide");
        });
        $("#btn-friend").unbind().click(function () {
            if (contacts[currentContact].category != "friend") {
                contacts[currentContact].category = "friend";
                Messenger.setCategory(currentContact, "friend");
                $(this).parent().find('.contact-category').html(TR('friend'));
                $("#btn-friend").text(TR('Remove from Friends'));
                $("#btn-black").text(TR('Add_to_blacklist'));
            } else {
                contacts[currentContact].category = "no category";
                Messenger.setCategory(currentContact, "no category");
                $(this).parent().find('.contact-category').html(TR('no category'));
                $("#btn-friend").text(TR('Add to friends'));
                $("#btn-black").text(TR('Add_to_blacklist'));
            }
            changeCategory(cVisible);
        });
        $("#btn-black").unbind().click(function () {
            if (contacts[currentContact].category != "blacklisted") {
                contacts[currentContact].category = "blacklisted";
                Messenger.setCategory(currentContact, "blacklisted");
                $(this).parent().find('.contact-category').html(TR('blacklisted'));
                $("#btn-black").text(TR('Remove from blacklist'));
                $("#btn-friend").text(TR('Add to friends'));
            } else {
                contacts[currentContact].category = "no category";
                Messenger.setCategory(currentContact, "no category");
                $(this).parent().find('.contact-category').html(TR('no category'));
                $("#btn-black").text(TR('Add_to_blacklist'));
                $("#btn-friend").text(TR('Add to friends'));
            }
            changeCategory(cVisible);
        });
        $("#btn-alias").unbind().click(function () {
            var alias = $("#edit-contact").find("input[name='contact-alias']").val();
            var r = Messenger.setAlias(currentContact, alias);
            contacts[currentContact].alias = alias;
            Messenger.saveContacts();
            Messenger.updateContactDisplay(currentContact);
            if (r)
                CPage.showNotice(TR("Successfully change alias to ") + alias);
            else
                CPage.showNotice(TR("Fail to change alias to ") + alias);
        });
        $('#msg-history').scroll(function () {
            if ($('#msg-history').scrollTop() == 0) {
                $('#msg-history').scrollTop(100);
                Messenger.showOldMessages();
            }
        });
        initAccount();
        changeCategory($("#list-all"));
        function resetCl() {
            var t = $(window).height() - $("#contact-list").offset().top;
            $("#contact-list").height(t);
        }
        $(window).resize(function () {
            resetC2();
            resetCl();
        });
        prepareStdTpl();
        resetC2();
        resetCl();
        if (page === "chatto") {
            var id = CUtil.getGet("chatto");
            if (!FAI_API.checkNameKey(id)) {
                CPage.showNotice(TR('ID is not valid'));
            } else {
                Messenger.addContact(id);
                Messenger.switchToContact(id);
            }
        }
        CPage.registerNotifications();
    });
});
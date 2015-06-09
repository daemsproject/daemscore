/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Messenger = new function() {
    var Messenger = this;
    var tx_page;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view    
    var accountID;    
    var transactions = []; //List of all transactions (initially populated from /multiaddr updated through websockets)    
    var contacts=[];
    var archTimer; //Delayed Backup wallet timer
    var currentContact;
    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)
    
    var isInitialized = false;    
    var language = 'en'; //Current language    
    var haveBoundReady = false;
    
    var sync_pubkeys = false;    
    var wallet_options = {    
        fee_policy : 0,  //Default Fee policy (-1 Tight, 0 Normal, 1 High)
        html5_notifications : false, //HTML 5 Desktop notifications    
        tx_display : 0, //Compact or detailed transactions    
        transactions_per_page : 1000, //Number of transactions per page    
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
    this.makeNotice = function(type, id, msg, timeout) {

        if (msg == null || msg.length == 0)
            return;

        console.log(msg);

        var el = $('<div class="alert alert-block alert-'+type+'"></div>');

        el.text(''+msg);

        if ($('#'+id).length > 0) {
            el.attr('id', id);
            return;
        }

        $("#notices").append(el).hide().fadeIn(200);

        (function() {
            var tel = el;

            setTimeout(function() {
                tel.fadeOut(250, function() {
                    $(this).remove();
                });
            }, timeout ? timeout : 5000);
        })();
    }
    function switchToContact(id){
        console.log("swtich");
    }
    function addToContactList(c){
        var html='<div id='+c.id+' onclick="switchToContact('+c.id+')">';
        if (c.image)
            html+=createImgHtml(c.image);
        if(c.alias)
        html+=c.alias;
        html+=" "+c.id+"</div>";
        $("#contact-list").prepend(html);
    }
    function readContacts(func){
        BrowserAPI.read_contacts(accountID,function(data){
            contacts=data;
            contacts[1]={id:"abc"};
            for(var i in contacts){
                addToContactList(contacts[i]);
            }
            if(contacts.length>0)
                currentContact=contacts[0].id;
            func();
        },function(e){
          console.log(e);                    
          Messenger.makeNotice('error', 'read-contacts-error',e);  
          func();
        });
        
    }
    function addMessage(msg){         
        
            
    }
    
    this.getMessages=function(){
        BrowserAPI.getMessages(accountID,null,function(data){
            if (!data||data.error){
                if (error) error();
                return;                
            }
            console.log(data);    
            
                for(var i in data)                    
                    addMessage(data[i]);
                //showRevenues();
           

        }, function() {
        });
    };
    
    function registerNotifications(){
        //var aa=fuction(a){(a);};
//        var aa=function(a){
//            MyWallet.notifiedBlock(a);
//        };
        var ab=function(a){
            Messenger.notifiedTx(a);
        };
        //BrowserAPI.regNotifyBlocks(aa);        
        BrowserAPI.regNotifyTxs(ab,[accountID]);
//        BrowserAPI.regNotifyAccount(this.notifiedAccount);
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.notifiedTx=function(a){
        console.log("notified tx");
            addMessages[a];
                       
    };
    
    this.notifiedAccount=function(data){
        
    }
    this.notifiedPeers=function(data){
        
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
    
    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        
        haveBoundReady = true;
        $("#add-contact").find(".btn-secondary").unbind().click(function() {               
            $("#add-contact").hide();
       });
        $("#add-contact").find(".btn-primary").unbind().click(function() {
            var arr=[{id:$("#add-contact").find('input[name="contact-id"]').val()}];
            //var contact=;
            //arr.concat();
            BrowserAPI.add_contacts(accountID,arr,function(a){
                
            },function(e){
                Messenger.makeNotice('error', 'add-contact-error',e);
            })
            $("#add-contact").hide();
       });    
        
        
        $('.btn-send').unbind().click(function() {               
            if(currentContact){
                
            }
       });
       $('#btn-add').unbind().click(function() { 
           $("#add-contact").show();
           $("#add-contact").center();
       });      
    }

    function initAccount(){
        accountID=BrowserAPI.getAccountID();          
        $("#account-id").html(accountID);
        //IDs=BrowserAPI.getIDs(accountID);  
        //BrowserAPI.getNewID(accountID);
        registerNotifications();
        //readContacts(Messenger.get_history);    
        Messenger.getMessages();
    }
    
    
   $(document).ready(function() {

        if (!$.isEmptyObject({}) || !$.isEmptyObject([])) {
            Messenger.makeNotice('error', 'error', 'Object.prototype has been extended by a browser extension. Please disable this extensions and reload the page.');
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

        
         
        bindInitial();  
        initAccount(); 
        bindReady();
        //Frame break
        if (top.location != self.location) {
            top.location = self.location.href
        }

       

        

        $(document).ajaxStart(function() {
                setLogoutImageStatus('loading_start');

                $('.loading-indicator').fadeIn(200);
            }).ajaxStop(function() {
                setLogoutImageStatus('loading_stop');

                $('.loading-indicator').hide();
            });
    });
}
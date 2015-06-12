/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var Messenger = new function() {
    var i = this;
    var tx_page;
    this.skip_init = false; //Set on sign up page
    var cVisible; //currently visible view    
    var accountID;    
    var transactions = []; //List of all transactions (initially populated from /multiaddr updated through websockets)    
    var contacts=[];
    var archTimer; //Delayed Backup wallet timer
    var currentContact;
    var event_listeners = []; //Emits Did decrypt wallet event (used on claim page)
    var messages=[];
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
    this.switchToContact=function(id){
        $("#current-contact").html(id);
        currentContact=id;
        i.showMessages(id);
    }
    function addToContactList(c){        
        if(contacts[c.id])
            return;
        contacts[c.id]=c;
        var html='<div id="'+c.id+'" onclick="{Messenger.switchToContact(\''+c.id+'\')}" style="margin-top:20px">';
        if (c.image)            
            html+=createImgHtml(c.image);
        if(c.alias)
        html+=c.alias;
        html+=" "+showID(c.id)+"<div class='nmsgs' style='float:right'></div></div>";
        console.log(html);
        $("#contact-list").append(html);
        //Messenger.switchToContact(c.id);
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
    this.showMessage=function(msg,direction){
        var html='<div style="text-align:'+direction+'">';
        if(direction=="left")
            html+="->";
        else
            html+="<-";
                html+=msg+'</div>';
        console.log(html);
        $('#history-message').append(html);
    }
    this.showMessages=function(id){
        $('#history-message').html(" ");
        var msgs=[];
        for(var j in messages[id]){
            msgs.push(messages[id][j].content);
        }
        console.log(msgs);
        i.decryptAndShow(id,msgs);
        
    }    
    this.decryptAndShow=function(id,msgs){
        BrowserAPI.decryptMessages(accountID,[{idForeign:id,messages:msgs}],function(decryptmsgs){
            console.log(decryptmsgs);        
            for(var j=decryptmsgs[0].messages.length-1;j>=0;j--){
                if(decryptmsgs[0].messages[j]){
                    console.log(messages[id][j].IDTo);
                    console.log(id);
                    console.log(BrowserAPI.areIDsEqual(messages[id][j].IDTo,id));
                    var direction=BrowserAPI.areIDsEqual(messages[id][j].IDTo,id)?"right":"left";                
                    i.showMessage(decryptmsgs[0].messages[j],direction);
                }
            }    
        },function(){});
    }
    this.addMessage=function(msg){         
        msg.IDForeign=BrowserAPI.areIDsEqual(msg.IDFrom,accountID)?msg.IDTo:msg.IDFrom;
        addToContactList({id:msg.IDForeign});
        if(!messages[msg.IDForeign])
            messages[msg.IDForeign]=[];
        messages[msg.IDForeign].push(msg);
        $("#"+msg.IDForeign).find(".nmsgs").html(messages[msg.IDForeign].length);
    }
    
    this.getMessages=function(){
        console.log("getMessages");         
        BrowserAPI.getMessages(accountID,null,0,true,false,0,1000,function(data){
            if (!data||data.error){
                if (error) error();
                return;                
            }
            console.log(data); 
                for(var i in data)                    
                    Messenger.addMessage(data[i]);
        }, function() {
            console.log("getMessage error"); 
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
        var ac=function(){
            window.location.href=window.location.href;
        }
        //BrowserAPI.regNotifyBlocks(aa);        
        BrowserAPI.regNotifyTxs(ab,[accountID]);
        BrowserAPI.regNotifyAccount(ac);
//        BrowserAPI.regNotifyPeers(this.notifiedPeers);
    }
    this.notifiedTx=function(a){
        console.log("notified tx");
        var data=BrowserAPI.getTxMessages(accountID,[a.tx.txid]);        
        if (data||!data.error){
            for(var i in data){   
                msg=data[i];                
               Messenger.addMessage(msg);        
            }
            if(BrowserAPI.areIDsEqual(msg.From,currentContact)&&BrowserAPI.areIDsEqual(msg.IDTo,accountID)){
                   i.decryptAndShow(currentContact,data);
           }
        }               
    };
    
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
            var id=$("#add-contact").find('input[name="contact-id"]').val();
            if(contacts[id]){
                Messenger.makeNotice('error', 'error', 'ID already in contact list');    
                return;
            }
            //console.log(BrowserAPI.checkNameKey(id));
            if(!BrowserAPI.checkNameKey(id)){
                 Messenger.makeNotice('error', 'error', 'ID is not valid');                 
                return;
            }
            
            addToContactList({id:id});
            
            //var arr=[{id:$("#add-contact").find('input[name="contact-id"]').val()}];
            
//            BrowserAPI.add_contacts(accountID,arr,function(a){
//                
//            },function(e){
//                Messenger.makeNotice('error', 'add-contact-error',e);
//            })
            $("#add-contact").hide();
       });    
        
        
        $('#btn-send').unbind().click(function() {   
            console.log($("#send-message-box").val());
            var msg=$("#send-message-box").val();
            if(currentContact&&msg){                
                BrowserAPI.sendMessage(accountID,currentContact,msg,function(){
                    i.showMessage(msg,"right");
                },function(e){
                    i.makeNotice('error', 'send-message-error', e);
           
                });
                    
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
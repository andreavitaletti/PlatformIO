//SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

contract Lottery{
    //manager is in charge of the contract
    address public manager;
    //new player in the contract using array[] to unlimit number
    address[] public players;

    constructor() {
        manager = msg.sender;
    }
    //to enter the lottery, players must pay a minimum entry fee and provide valid temperature and humidity
    function enter(uint temp, uint hum) public payable{
        require(msg.value > .01 ether, "insufficient entry fee");
        require(temp < 40, "temperature too high");
        require(hum > 0, "invalid humidity");
        players.push(msg.sender);
    }

    function getContractBalance() public view returns(uint){
        return address(this).balance;
    }

    //creates a random hash that will become our winner
    function random() private view returns(uint){
        return  uint (keccak256(abi.encode(block.timestamp,  players)));
    }

    function pickWinner() public restricted{
        //only the manager can pickWinner
        require(players.length > 0, "no players");
        //creates index that is gotten from func random % play.len
        uint index = random() % players.length;
        //pays the winner picked randomely(not fully random)
        (bool success, ) = payable(players[index]).call{value: address(this).balance}("");
        require(success, "payout failed");
        //empies the old lottery and starts new one
        players = new address[](0);
    }

    modifier restricted(){
        require(msg.sender == manager);
        _;

    }
}

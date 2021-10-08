# ZombieSurvivingAI

A second year exam project for the gameplay programming course at DAE (http://www.digitalartsandentertainment.be/)



This project consist of the implementation of an AI whose goal is too score the most points possible in a zombie survival game.



<h3 class="has-dark-gray-color has-text-color has-medium-font-size">Gameplay Mechanics</h3>
<!-- /wp:heading -->

<!-- wp:heading {"level":4} -->
<h4>Game layout</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>The game area is a square in which are placed a few houses where four types of items can spawn. Those types are <em>Pistol</em>, <em>EnergyKit</em>, <em>Medkit</em> and <em>Garbage</em>. While the threefirst  have a limited amount of uses, the last is unusable by the character.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>The character itself has a field of view that allows it to perceive items, houses and enemies. For the items, the character doesn't know what type it is until he picks one up. Once an item is picked up, it is directly placed in the character inventory which consists of 5 slots.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>The game consists of multiple stages, each more difficult than the last. Every stage lasts 60 seconds and has its own number and composition. Stronger enemied will become more likely to spawn in later stages and useful items less likely. Starting from stages 3, <em>"Purge Zones"</em> will start to spawn. If the character stands in them for too long, he will be killed instantly.</p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":4} -->
<h4>Scoring</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>The score is based on a reward and penalty system :</p>
<!-- /wp:paragraph -->

<!-- wp:list -->
<ul><li>Positive score is added for the following actions:<ul><li>Each second survived : 1 point.</li><li>Killing a zombie : 15 points.</li><li>Hitting a zombie : 5 points.</li><li>Picking up an item : 2 points.</li></ul></li><li>Negative score is given for the following action:<ul><li>Missing a shot : -5 points.</li></ul></li></ul>
<!-- /wp:list -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":3,"textColor":"dark-gray"} -->
<h3 class="has-dark-gray-color has-text-color">The AI </h3>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>Thsi AI is implemented using a behavior tree managing the priority of each of its different behaviors described on the following lines.</p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":4} -->
<h4>World exploration</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>The character starts without any knowledge of his environment and has to start by exploring. In order to make its exploration easier, the world is partitioned into several areas visualized as red squares in the project. The character then just go to the closest unexplored area.</p>
<!-- /wp:paragraph -->

<!-- wp:image {"align":"center","id":51,"width":390,"height":391,"sizeSlug":"large","linkDestination":"none"} -->
<div class="wp-block-image"><figure class="aligncenter size-large is-resized"><img src="https://maximeleriche.files.wordpress.com/2021/10/partitioned-space.png?w=486" alt="" class="wp-image-51" width="390" height="391"/></figure></div>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p> He is then able to learns the position of houses in areas and once all are explored he can then look for the closest house in his area or neighboring ones.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>When spotting a house, exploring it becomes a priority over world exploration as in houses are spawned items.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>When a house is entered, it gets the tag <em>"Explored"</em> for a minute. After this time, the character will be able to explore it again to see if new items spawned.</p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":4} -->
<h4>Steering behaviors</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>The game area has a navmesh that the character and enemies use to avoid collision with the houses walls.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>The main steering behavior used is "Seek" combined with an obstacle avoidance algorithm. In this case the obstacles are the enemies. The player constantly seeks for the center of an area, a house or an item.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>The other behavior used is "Face" to align with an enemy in order to shoot it.</p>
<!-- /wp:paragraph -->

<!-- wp:image {"id":53,"sizeSlug":"large","linkDestination":"none"} -->
<figure class="wp-block-image size-large"><img src="https://maximeleriche.files.wordpress.com/2021/10/movement.gif?w=1024" alt="" class="wp-image-53"/></figure>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":4} -->
<h4>Items handling</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>Seeking items is the absolute priority over exploration and they will be used as soon as needed (need of health or energy etc.)</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>When the inventory is full, the character will not pick up the item but remember its position in order to come back to it when needed.</p>
<!-- /wp:paragraph -->

<!-- wp:image {"id":55,"sizeSlug":"large","linkDestination":"none"} -->
<figure class="wp-block-image size-large"><img src="https://maximeleriche.files.wordpress.com/2021/10/itemhandling.gif?w=1024" alt="" class="wp-image-55"/></figure>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":4} -->
<h4>Enemies handling</h4>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>The character doesn't flee from enemies, he will instead avoid them thans to the obstacl avoidance steering behavior. </p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>When colliding with an enemy (thus being bitten and taking damage) the character will start sprinting (at the cost of energy) in order to put distance between him and the zombie and hopefully getting out of his aggro range.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p>If the player is armed with the <em>pistol</em> item shooting enemies in range is the absolute priority over any other behaviour.</p>
<!-- /wp:paragraph -->

<!-- wp:image {"id":57,"sizeSlug":"large","linkDestination":"none"} -->
<figure class="wp-block-image size-large"><img src="https://maximeleriche.files.wordpress.com/2021/10/enemyhandling.gif?w=1024" alt="" class="wp-image-57"/></figure>
<!-- /wp:image -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":3,"textColor":"dark-gray"} -->
<h3 class="has-dark-gray-color has-text-color">Conclusion and future work</h3>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p>This AI was my proposal for the final exam of the gameplay programming course at Digital Arts and Entertainment school. I plan to extend on it by implementing it in Unity or Unreal and making the AI even mor performant by adding behavior unused here as combined steering for example.</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

<!-- wp:heading {"level":3} -->
<h3>Resources</h3>
<!-- /wp:heading -->

<!-- wp:paragraph -->
<p><em>Artificial intelligence for games</em> by Ian Millington &amp; John Funge</p>
<!-- /wp:paragraph -->

<!-- wp:paragraph -->
<p></p>
<!-- /wp:paragraph -->

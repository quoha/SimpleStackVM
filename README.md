Using XCode with github
=======================

0. These steps were pulled from the documentation on github

1. Create an empty repository on GitHub  
    do not initialize with a README

2. Note the SSH path to the new repository  
    git@github.com:quoha/derp-octo-sansa.git

3. Create a project in XCode with local git repository  
    /Users/mdhender/Software/derpOctoSansa

4. CD to your project folder  
    $ cd /Users/mdhender/Software/derpOctoSansa

5. Create a .gitignore file  
    There is a good one at https://gist.github.com/3786883  
    I like to add *.xcworkspace  
    $ git add .gitignore  

6. Create a README.md (because github likes it)
    $ touch README.md  
    $ git add README.md  

7. Commit the changes and push to github  
    $ git commit -m "sync github with local xcode"  
    $ git remote add origin git@github.com:quoha/derp-octo-sansa.git  
    $ git push -u origin master  

8. Use Option-Command-C to test a commit from XCode

9. Test a push from XCode  
    Menu File -> Source Control -> Push  
    Wait for the dialog to populate  
        Chose the remote to which to push changes  
        Remote: origin / master  
        * Repository is online  
    Click the Push button  

10. Go out to github and verify

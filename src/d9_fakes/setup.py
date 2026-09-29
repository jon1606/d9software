from setuptools import find_packages, setup

package_name = 'd9_fakes'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Jonathan',
    maintainer_email='yonathan.transky1@gmail.com',
    description='Stand-ins for the other teams: fake_server, fake_drone, fake_hq.',
    license='TBD',
    entry_points={'console_scripts': []},
)
